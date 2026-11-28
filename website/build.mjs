import {execFileSync} from 'node:child_process';
import {cpSync, existsSync, mkdirSync, readFileSync, writeFileSync} from 'node:fs';
import {dirname, posix, resolve} from 'node:path';
import {fileURLToPath} from 'node:url';
import {JSDOM} from 'jsdom';
import {blobURL, BRANCH, HOME, makeIndex} from './assets/source.mjs';
import {parseNavigation} from './assets/navigation.mjs';
import {priority, ui} from './assets/ui.mjs';

const source = fileURLToPath(new URL('../', import.meta.url));
const out = resolve(process.argv[2] || fileURLToPath(new URL('dist/', import.meta.url)));
const git = (...args) => execFileSync('git', ['-C', source, ...args], {encoding:'utf8'});
let index = makeIndex(git('rev-parse', 'HEAD').trim(), git('ls-files', '-z').split('\0'));
if (existsSync(resolve(source, 'docs/site.nav'))) {
  const nav = parseNavigation(readFileSync(resolve(source, 'docs/site.nav'), 'utf8'), index.entries);
  if (nav.warnings.length) console.warn('Invalid docs/site.nav lines:', nav.warnings.join(', '));
  index = {...index, ...nav};
}

const template = readFileSync(new URL('index.html', import.meta.url), 'utf8');
const renderer = new JSDOM('');
globalThis.window = renderer.window;
globalThis.Node = renderer.window.Node;
const {renderMarkdown, element, link} = await import('./assets/render.mjs');
const shell = new JSDOM(template);
const site = shell.window.document.querySelector('[rel="canonical"]').href;
shell.window.close();
const routes = new Map();
for (const e of index.entries) {
  const folder = e.key === 'home' ? (e.lang === 'en' ? '' : 'fr/')
    : e.path.startsWith('docs/' + e.lang + '/') ? e.path.slice(5, -3) + '/'
    : e.lang + '/project/' + posix.dirname(e.path) + '/';
  const path = folder.split('/').map(encodeURIComponent).join('/');
  if ([...routes.values()].includes(path)) throw new Error('Duplicate URL: ' + path);
  routes.set(e.path, path);
}
function local(from, to) {
  const path = posix.relative('/' + from, '/' + to);
  return path ? path + (to.endsWith('/') || !to ? '/' : '') : './';
}
function write(path, data) {
  const file = resolve(out, path);
  mkdirSync(dirname(file), {recursive:true});
  writeFileSync(file, data);
}

for (const page of index.entries) {
  const {lang, path, key} = page;
  const dest = routes.get(path);
  const canonical = new URL(dest, site).href;
  const dom = new JSDOM(template);
  const doc = dom.window.document;
  globalThis.document = doc;
  const $ = id => doc.getElementById(id);
  const t = ui[lang];
  const groups = [...(index.sections?.[lang] || []), ...Object.entries(t.groups)];
  const pageURL = (_lang, target, section) =>
    local(dest, routes.get(target)) + (section ? '#section-' + encodeURIComponent(section) : '');
  doc.documentElement.lang = lang;
  const home = key === 'home';
  const {dom:article, headings} = renderMarkdown(readFileSync(resolve(source,path), 'utf8'), {
    path, lang, sha:index.sha, pages:index.entries, home, labels:t, pageURL,
  });
  const heading = article.querySelector('h1');
  if (home && heading) heading.textContent = lang === 'fr' ? 'Langage de programmation Prysma' : 'Prysma Programming Language';
  const label = page.navTitle || t.names[key] || heading?.textContent || page.title;
  if (!heading) article.prepend(element('h1', label));
  doc.title = home
    ? (lang === 'fr' ? 'Prysma : langage et compilateur LLVM | Documentation officielle' : 'Prysma Language and LLVM Compiler | Official Documentation')
    : label + ' | Prysma';
  const intro = [...article.querySelectorAll('p')].map(p => p.textContent.trim()).find(text => text.length >= 35);
  if (!home || lang === 'fr') {
    const description = intro ? intro.replace(/\s+/g,' ').slice(0, 155).trimEnd()
      : (lang === 'fr' ? 'Documentation Prysma : ' : 'Prysma documentation: ') + label;
    doc.querySelector('meta[name="description"]').content = description;
  }
  doc.querySelector('link[rel="canonical"]').href = canonical;
  if (dest) doc.querySelector('meta[name="google-site-verification"]')?.remove();
  doc.querySelector('script[type="module"]').src = local(dest, 'assets/static.mjs');
  for (const asset of doc.querySelectorAll('link[href^="assets/"], img[src^="assets/"]')) {
    const attr = asset.hasAttribute('src') ? 'src' : 'href';
    asset.setAttribute(attr, local(dest, asset.getAttribute(attr)));
  }
  $('brand').href = pageURL(lang, HOME[lang]);
  $('skip').textContent = t.skip;
  $('menu').textContent = t.browse;
  $('search-label').textContent = t.find;
  $('search').placeholder = t.find + '…';
  $('nav').setAttribute('aria-label', t.docs);
  $('outline').setAttribute('aria-label', t.outline);
  $('outline-title').textContent = t.outline;
  $('footer-license').textContent = t.license;
  $('crumb').textContent = t.docs + ' / ' + (groups.find(([group]) => page.group === group)?.[1] || t.page);
  $('edit-source').href = blobURL(BRANCH,path);
  $('edit-source').textContent = t.source;
  $('edit-source').hidden = false;
  $('source-status').textContent = BRANCH + '@' + index.sha.slice(0,7);
  $('document').removeAttribute('aria-busy');
  doc.querySelector('noscript').remove();
  $('article').replaceChildren(article);
  $('outline-links').replaceChildren(...headings.map(h => link(h.text, '#section-' + encodeURIComponent(h.id), h.sub?'sub':'')));
  $('outline').hidden = headings.length === 0;
  $('outline-details').open = true;
  for (const target of ['fr','en']) {
    const translation = index.entries.find(e => e.key === key && e.lang === target);
    $(target).href = pageURL(target, translation?.path || HOME[target]);
    if (target === lang) $(target).setAttribute('aria-current','true');
    if (!translation) continue;
    const meta = element('link');
    meta.rel = 'alternate';
    meta.hreflang = target;
    meta.href = new URL(routes.get(translation.path), site).href;
    doc.head.append(meta);
  }
  for (const [group, caption] of groups) {
    const pages = index.entries.filter(e => e.group === group && e.lang === lang);
    if (!pages.length) continue;
    pages.sort((a,b) => (a.order ?? priority[a.key] ?? 99) - (b.order ?? priority[b.key] ?? 99) || a.title.localeCompare(b.title,lang));
    const section = element('section', '', 'nav-group');
    section.append(element('h2', caption));
    for (const e of pages) {
      const navLink = link(e.navTitle || (e.key === 'home' ? t.page : t.names[e.key] || e.title), pageURL(lang,e.path));
      if (e.path === path) navLink.setAttribute('aria-current','page');
      section.append(navLink);
    }
    $('nav').append(section);
  }
  // GitHub READMEs point to the documentation site, not back to themselves.
  if (home) for (const a of $('article').querySelectorAll('a')) {
    if (a.getAttribute('href') === site + '#' + lang + '/' + HOME[lang]) {
      if (a.parentElement?.matches('p') && a.parentElement.textContent.trim() === a.textContent.trim()) a.parentElement.remove();
    }
  }
  write(decodeURIComponent(dest) + 'index.html', dom.serialize());
  dom.window.close();
}
const escapeXML = s => s.replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;');
const nodes = [...routes.values()].map(p => '  <url><loc>' + escapeXML(new URL(p,site).href) + '</loc></url>');
write('sitemap.xml', '<?xml version="1.0" encoding="UTF-8"?>\n<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">\n' + nodes.join('\n') + '\n</urlset>\n');
write('routes.json', JSON.stringify(Object.fromEntries(routes)) + '\n');
cpSync(new URL('assets/', import.meta.url), resolve(out,'assets'), {recursive:true});
renderer.window.close();
console.log('Published ' + routes.size + ' documentation pages.');
