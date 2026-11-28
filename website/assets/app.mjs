import {BRANCH, GITHUB, HOME, blobURL, rawURL, request, loadMain, fold, routeURL, readRoute} from './source.mjs';
import {element, link, renderMarkdown} from './render.mjs';

const ui = {
  en:{
    docs:'Documentation', browse:'Browse docs', find:'Find a page', outline:'On this page', skip:'Skip to content',
    groups:{start:'Overview',setup:'Set up Prysma',language:'Language & compiler',project:'Project',operations:'Infrastructure'},
    source:'View source', page:'Overview', git:'Prysma on GitHub',
    names:{'installation/first_program.md':'First program','installation/docker_compilation.md':'Build with Docker','installation/vscode_environment.md':'VS Code and debugging','installation/setup_ubuntu.md':'Native Ubuntu build','prysma.md':'Language reference','architecture.md':'Compiler architecture','changelog.md':'Changelog','roadmap.md':'Roadmap','robustness.md':'Robustness and security'},
    loading:'Loading documentation…', latest:'Synced with GitHub', failed:'GitHub unavailable', cached:'Last known revision',
    empty:'No matching pages.', found:n=>n+' pages found', unavailable:'This page could not be loaded. Open the source on GitHub or return to the overview.',
    missing:'This page is not in the current documentation.', license:'GPL-3.0-or-later with Prysma exception',
    copy:'Copy', copied:'Copied',copyError:'Unavailable',table:'Scrollable table', navError:'Invalid navigation lines:', navUnavailable:'Navigation file unavailable',
    alerts:{note:'Note',tip:'Tip',important:'Important',warning:'Warning',caution:'Caution'},
  },
  fr:{
    docs:'Documentation', browse:'Parcourir la doc', find:'Trouver une page', outline:'Sur cette page', skip:'Aller au contenu',
    groups:{start:'Vue d’ensemble',setup:'Installer Prysma',language:'Langage et compilateur',project:'Projet',operations:'Infrastructure'},
    source:'Voir la source',page:'Vue d’ensemble',git:'Prysma sur GitHub',
    names:{'installation/first_program.md':'Premier programme','installation/docker_compilation.md':'Compiler avec Docker','installation/vscode_environment.md':'VS Code et débogage','installation/setup_ubuntu.md':'Compilation native sur Ubuntu','prysma.md':'Référence du langage','architecture.md':'Architecture du compilateur','changelog.md':'Journal de développement','roadmap.md':'Feuille de route','robustness.md':'Robustesse et sécurité'},
    loading:'Chargement de la documentation…',latest:'Synchronisé avec GitHub',failed:'GitHub indisponible',cached:'Dernière révision connue',
    empty:'Aucune page ne correspond.',found:n=>n+' pages trouvées',unavailable:'Impossible de charger cette page. Ouvre la source sur GitHub ou reviens à la vue d’ensemble.',
    missing:'Cette page ne figure pas dans la documentation actuelle.',license:'GPL-3.0-or-later avec exception Prysma',
    copy:'Copier',copied:'Copié',copyError:'Indisponible',table:'Tableau défilant', navError:'Lignes de navigation invalides :', navUnavailable:'Fichier de navigation indisponible',
    alerts:{note:'Note',tip:'Conseil',important:'Important',warning:'Avertissement',caution:'Attention'},
  },
};
const $ = id => document.getElementById(id);
const q = $('search');
const cacheAge = 5 * 60 * 1000;
const compactOutline = matchMedia('(max-width:1100px)');
let index, sourceState = 'latest', active, renderId = 0, syncBusy = false, lastCheck = 0;
let visiblePage = '';
const texts = new Map();
const preferredLanguage = () => /^fr\b/i.test(navigator.language) ? 'fr' : 'en';
function localLabels(lang) {
  const t = ui[lang];
  document.documentElement.lang = lang;
  $('brand').href = routeURL(lang);
  $('skip').textContent = t.skip;
  $('menu').textContent = t.browse;
  $('search-label').textContent = t.find;
  $('nav').setAttribute('aria-label', t.docs);
  $('outline').setAttribute('aria-label',t.outline);
  q.placeholder = t.find + '…';
  $('footer-license').textContent = t.license;
}
function sourceLabel(lang) {
  const t = ui[lang];
  const summary = sourceState === 'cached' ? t.failed + ' · ' + t.cached : t[sourceState];
  const nav = index?.navUnavailable ? ' · ' + t.navUnavailable
    : index?.warnings?.length ? ' · ' + t.navError + ' ' + index.warnings.join(', ') : '';
  $('source-status').textContent = summary + (index ? ' · ' + BRANCH + '@' + index.sha.slice(0,7) : '') + nav;
}
const priority = {'home':0,'installation/first_program.md':0,'installation/docker_compilation.md':1,
  'installation/vscode_environment.md':2,'installation/setup_ubuntu.md':3,'prysma.md':0,'architecture.md':1,'changelog.md':0,'roadmap.md':1,
  'installation/dashboard/guide.md':0};
function navigation(lang, path) {
  const t = ui[lang];
  const terms = fold(q.value.trim()).split(/\s+/).filter(Boolean);
  const items = index.entries.filter(p => p.lang === lang && terms.every(word => fold(p.title + ' ' + (p.navTitle || t.names[p.key] || '') + ' ' + p.key.replace(/[_.\/\-]/g, ' ')).includes(word)));
  const children = [];
  for (const [group,caption] of [...(index.sections?.[lang] || []), ...Object.entries(t.groups)]) {
    const list = items.filter(p => p.group === group);
    if (!list.length) continue;
    list.sort((a,b) => (a.order ?? priority[a.key] ?? 99) - (b.order ?? priority[b.key] ?? 99) || a.title.localeCompare(b.title,lang));
    const section = element('section','','nav-group');
    section.append(element('h2',caption));
    for (const entry of list) {
      const a = link(entry.navTitle || (entry.key === 'home' ? t.page : (t.names[entry.key] || entry.title)), routeURL(lang,entry.path));
      if (entry.path === path) a.setAttribute('aria-current','page');
      section.append(a);
    }
    children.push(section);
  }
  $('nav').replaceChildren(...(children.length ? children : [element('p',t.empty,'notice')]));
  $('filter-status').textContent = terms.length ? t.found(items.length) : '';
}
function languageLinks(lang, entry) {
  for (const target of ['fr','en']) {
    const a = $(target);
    const translation = index.entries.find(p => p.lang === target && p.key === entry?.key);
    a.href = routeURL(target, translation?.path || HOME[target]);
    if (target === lang) a.setAttribute('aria-current','true'); else a.removeAttribute('aria-current');
  }
}
async function markdown(path, rev) {
  const key = rev.sha + ':' + path;
  if (texts.has(key)) return texts.get(key);
  const pending = request(rawURL(rev.sha,path), {signal:AbortSignal.timeout(8000)})
    .then(response => response.text());
  texts.set(key,pending);
  pending.catch(() => {if (texts.get(key) === pending) texts.delete(key);});
  return pending;
}
function jump(section) {
  if (!section) return false;
  const destination = $('section-' + section);
  destination?.scrollIntoView();
  return Boolean(destination);
}
async function render({preserveScroll = false} = {}) {
  if (!index) return;
  const route = readRoute(location.hash, preferredLanguage());
  const {lang,path,section} = route;
  const t = ui[lang], version = index;
  const key = version.sha + ':' + lang + ':' + path;
  const samePage = active?.lang === lang && active?.path === path && visiblePage === key;
  if (active?.lang !== lang) q.value = '';
  active = route;
  if (samePage && !preserveScroll && jump(section)) return;
  const ticket = ++renderId;
  const entry = version.entries.find(item => item.path === path && item.lang === lang);
  const home = entry?.key === 'home';
  localLabels(lang);
  navigation(lang,path);
  languageLinks(lang,entry);
  sourceLabel(lang);
  $('crumb').textContent = t.docs + (entry ? ' / ' + ((version.sections?.[lang] || []).find(([id]) => id === entry.group)?.[1] || t.groups[entry.group]) : '');
  document.title = home ? 'Prysma · ' + t.docs : (entry?.title || t.docs) + ' · Prysma';
  $('document').setAttribute('aria-busy','true');
  $('edit-source').hidden = true;
  $('outline').hidden = true;
  const article = $('article');
  if (!preserveScroll) article.replaceChildren(element('p',t.loading));
  try {
    if (!entry) {
      visiblePage = '';
      article.replaceChildren(element('p',t.missing,'notice'),link(t.page,routeURL(lang)));
      return;
    }
    const content = await markdown(path,version);
    if (ticket !== renderId) return;
    const {dom,headings} = renderMarkdown(content,{path,lang,sha:version.sha,pages:version.entries,home,labels:t});
    article.replaceChildren(dom);
    $('edit-source').href = blobURL(BRANCH,path);
    $('edit-source').textContent = t.source;
    $('edit-source').hidden = false;
    $('outline-title').textContent = t.outline;
    $('outline-details').open = !compactOutline.matches;
    $('outline-links').replaceChildren(...headings.map(h=>link(h.text,routeURL(lang,path,h.id),h.sub?'sub':'')));
    $('outline').hidden = headings.length === 0;
    visiblePage = key;
    if (!jump(section) && !preserveScroll) window.scrollTo(0,0);
  } catch {
    if (ticket !== renderId) return;
    visiblePage = '';
    article.replaceChildren(element('p', t.unavailable, 'notice'),link(t.source,blobURL(BRANCH,path)));
  } finally {if (ticket === renderId) $('document').setAttribute('aria-busy','false');}
}
async function refresh(force = false) {
  if (syncBusy || (!force && Date.now() - lastCheck < cacheAge)) return;
  syncBusy = true;
  lastCheck = Date.now();
  try {
    const first = !index;
    const current = await loadMain(index, {signal:AbortSignal.timeout(12000),cache:'no-cache'});
    const changed = current !== index;
    index = current;
    sourceState = 'latest';
    if (changed) {
      texts.clear();
      await render({preserveScroll: !first});
    } else sourceLabel(active?.lang || preferredLanguage());
  } catch {
    sourceState = index ? 'cached' : 'failed';
    if (index) sourceLabel(active?.lang || preferredLanguage());
    else failToLoad();
  } finally {syncBusy = false;}
}
function failToLoad() {
  const route = readRoute(location.hash, preferredLanguage()), t = ui[route.lang];
  localLabels(route.lang);
  sourceLabel(route.lang);
  $('article').replaceChildren(element('p', t.unavailable, 'notice'),
    link(t.git, blobURL(BRANCH, route.path)));
  $('document').setAttribute('aria-busy','false');
}
async function start() {
  const {lang} = readRoute(location.hash, preferredLanguage());
  localLabels(lang);
  $('article').replaceChildren(element('p',ui[lang].loading));
  $('source-status').textContent = ui[lang].loading;
  await refresh(true);
}
window.addEventListener('hashchange', () => { $('menu').setAttribute('aria-expanded','false'); if (index) void render(); else if(sourceState === 'failed') failToLoad(); });
window.addEventListener('online', () => void refresh(true));
compactOutline.addEventListener('change',() => { $('outline-details').open = !compactOutline.matches; });
$('outline-links').addEventListener('click', e => {if (compactOutline.matches && e.target.closest('a')) $('outline-details').open = false;});
document.addEventListener('visibilitychange', () => {if (!document.hidden) void refresh();});
setInterval(() => {if (!document.hidden) void refresh();}, cacheAge);
$('menu').addEventListener('click',()=> {
  const menu = $('menu');
  menu.setAttribute('aria-expanded', menu.getAttribute('aria-expanded') === 'false');
});
$('skip').addEventListener('click', e => {e.preventDefault(); $('document').focus();});
q.addEventListener('input',()=> {if(active) navigation(active.lang,active.path);});
q.addEventListener('keydown', e => {
  if(e.key === 'Enter') {const first = $('nav').querySelector('a'); if(first) location.hash = first.hash;}
  if(e.key === 'Escape') {q.value=''; if(active) navigation(active.lang,active.path); q.blur();}
});
document.addEventListener('keydown', e => {
  if(e.key !== '/' || e.ctrlKey || e.metaKey || e.altKey || e.target.closest('input,textarea,select,[contenteditable]')) return;
  e.preventDefault(); $('menu').setAttribute('aria-expanded','true'); q.focus();
});
void start();
