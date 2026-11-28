import {parseNavigation} from './navigation.mjs';

export const REPO = 'prysma-llvm/prysma';
export const BRANCH = 'main';
export const GITHUB = 'https://github.com/' + REPO;
const API = 'https://api.github.com/repos/' + REPO;
const RAW = 'https://raw.githubusercontent.com/' + REPO;
export const HOME = {en: 'README.md', fr: 'README.FR.md'};
export const SHA = /^[a-f0-9]{40}$/;
const encodePath = path => path.split('/').map(encodeURIComponent).join('/');
export const rawURL = (ref, path) => RAW + '/' + ref + '/' + encodePath(path);
export const blobURL = (ref, path) => GITHUB + '/blob/' + ref + '/' + encodePath(path);
const NAV_FILE = 'docs/site.nav';

const equivalents = {
  'language_specification.md': 'prysma.md',
  'robustness_security.md': 'robustness.md',
};
export function classify(path) {
  if (path === HOME.en || path === HOME.fr) return {key: 'home', lang: path === HOME.fr ? 'fr' : 'en', group: 'start'};
  const doc = /^docs\/(en|fr)\/(.+)\.md$/i.exec(path);
  if (doc) {
    const [,lang, stem] = doc;
    const key = equivalents[stem + '.md'] || stem + '.md';
    const group = stem.startsWith('installation/dashboard/') ? 'operations'
      : stem.startsWith('installation/') ? 'setup'
      : /^(prysma|language_specification|architecture|llvm_documentation\/)/.test(stem) ? 'language' : 'project';
    return {lang, key, group};
  }
  const readme = /^(.+)\/readme(\.(en|fr))?\.md$/i.exec(path);
  if (readme && !/^(?:website|\.github)\//i.test(path)) {
    return {lang: (readme[3] || 'en').toLowerCase(), key: readme[1] + '/README.md', group: 'operations'};
  }
  return null;
}
export function titleFromPath(path) {
  const parts = path.replace(/^docs\/(en|fr)\//, '').split('/');
  let name = parts.pop().replace(/\.md$/i, '');
  if (/^readme/i.test(name)) name = parts.pop() || 'Prysma';
  name = name.replace(/[_-]+/g, ' ');
  return name.charAt(0).toUpperCase() + name.slice(1);
}
export function makeIndex(sha, paths) {
  if (!SHA.test(sha)) throw new Error('Invalid source revision');
  const entries = [...new Set(paths)].sort().flatMap(path => {
    const meta = classify(path);
    return meta ? [{path, ...meta, title: titleFromPath(path)}] : [];
  });
  if (!Object.values(HOME).every(path => entries.some(e => e.path === path))) throw new Error('Missing bilingual READMEs');
  return {sha, entries};
}
export async function request(url, {fetcher = fetch, signal, ...options} = {}) {
  const response = await fetcher(url, {signal, ...options});
  if (!response.ok) throw new Error('HTTP ' + response.status + ' · ' + new URL(url, 'http://localhost').pathname);
  return response;
}
export async function loadMain(known, options = {}) {
  const head = await (await request(API + '/commits/' + BRANCH, options)).json();
  if (!SHA.test(head.sha) || !SHA.test(head.commit?.tree?.sha)) throw new Error('Unexpected GitHub revision');
  if (head.sha === known?.sha && !known.navUnavailable) return known;
  const tree = await (await request(API + '/git/trees/' + head.commit.tree.sha + '?recursive=1', options)).json();
  if (tree.truncated || !Array.isArray(tree.tree)) throw new Error('Incomplete GitHub tree');
  const paths = tree.tree.filter(x => x.type === 'blob' && /^100(?:644|755)$/.test(x.mode)).map(x => x.path);
  const index = makeIndex(head.sha, paths);
  if (!paths.includes(NAV_FILE)) return index;
  try {
    const nav = await (await request(rawURL(index.sha, NAV_FILE), options)).text();
    return {...index, ...parseNavigation(nav, index.entries)};
  } catch {
    return {...index, navUnavailable: true};
  }
}
export function routeURL(lang, path = HOME[lang], section = '') {
  return '#' + lang + '/' + encodePath(path) + (section ? '?section=' + encodeURIComponent(section) : '');
}
export function readRoute(hash, preferred = 'en') {
  try {
    const [route, query] = hash.slice(1).split('?');
    const [selected, ...encoded] = route.split('/');
    const lang = selected in HOME && ['en','fr'].includes(selected) ? selected : preferred;
    return {lang, path: encoded.length ? encoded.map(decodeURIComponent).join('/') : HOME[lang],
      section: new URLSearchParams(query).get('section') || ''};
  } catch { return {lang: preferred, path: HOME[preferred], section: ''}; }
}
export function fold(value) {
  return value.normalize('NFD').replace(/\p{Mark}/gu, '').toLowerCase();
}
export function slug(value) {
  return value.toLowerCase().replace(/[^\p{Letter}\p{Number}_\-\s]/gu, '').trim().replace(/\s/g, '-');
}
export function resolveRelative(path, href) {
  const base = 'https://repository.invalid/';
  try {
    const target = new URL(href, base + encodePath(path));
    if (target.origin !== new URL(base).origin) return null;
    return {path: decodeURIComponent(target.pathname.slice(1)), section: decodeURIComponent(target.hash.slice(1)), search: target.search};
  } catch { return null; }
}
