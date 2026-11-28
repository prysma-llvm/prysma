import {fold, readRoute} from './source.mjs';

const root = new URL('../', import.meta.url);
if (/^#(?:en|fr)(?:\/|$)/.test(location.hash)) {
  const {path, section} = readRoute(location.hash);
  try {
    const response = await fetch(new URL('routes.json', root));
    if (!response.ok) throw new Error('Unable to load documentation routes');
    const routes = await response.json();
    if (Object.hasOwn(routes, path)) {
      const page = new URL(routes[path], root);
      if (section) page.hash = 'section-' + section;
      if (page.href !== location.href) location.replace(page.href);
    }
  } catch {
    // Keep the current document usable if the legacy link cannot be resolved.
  }
}

const menu = document.getElementById('menu');
const search = document.getElementById('search');
const nav = document.getElementById('nav');
const groups = [...nav.querySelectorAll('.nav-group')];
menu.addEventListener('click', () => {
  menu.setAttribute('aria-expanded', menu.getAttribute('aria-expanded') === 'false');
});
search.addEventListener('input', () => {
  const terms = fold(search.value.trim()).split(/\s+/).filter(Boolean);
  for (const group of groups) {
    const links = [...group.querySelectorAll('a')];
    for (const a of links) a.hidden = !terms.every(term => fold(a.textContent).includes(term));
    group.hidden = links.every(a => a.hidden);
  }
  const count = nav.querySelectorAll('a:not([hidden])').length;
  const french = document.documentElement.lang === 'fr';
  document.getElementById('filter-status').textContent = french ? count + ' pages trouvées' : count + ' pages found';
});
search.addEventListener('keydown', event => {
  if (event.key === 'Enter') nav.querySelector('a:not([hidden])')?.click();
  if (event.key === 'Escape') {
    search.value = '';
    search.dispatchEvent(new Event('input'));
    search.blur();
  }
});
document.addEventListener('click', async event => {
  const button = event.target.closest('.copy');
  if (!button) return;
  const label = button.textContent;
  const french = document.documentElement.lang === 'fr';
  try {
    await navigator.clipboard.writeText(button.parentElement.querySelector('code').textContent);
    button.textContent = french ? 'Copié' : 'Copied';
  } catch {
    button.textContent = french ? 'Indisponible' : 'Unavailable';
  }
  setTimeout(() => { button.textContent = label; }, 1800);
});
