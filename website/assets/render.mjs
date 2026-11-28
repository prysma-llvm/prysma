import {marked} from './vendor/marked.mjs';
import DOMPurify from './vendor/dompurify.mjs';
import {GITHUB, REPO, rawURL, blobURL, resolveRelative, routeURL, slug} from './source.mjs';

export function element(tag, text, className) {
  const node = document.createElement(tag);
  if (text) node.textContent = text;
  if (className) node.className = className;
  return node;
}
export function link(text, href, className) {
  const node = element('a', text, className);
  node.href = href;
  return node;
}
function repoTarget(path, href) {
  if (!/^(?:[a-z][a-z\d+.-]*:|\/\/)/i.test(href)) return resolveRelative(path, href);
  try {
    const url = new URL(href, GITHUB);
    if (url.origin !== 'https://github.com') return null;
    const match = /^\/([^/]+\/[^/]+)\/(?:blob|tree)\/main(?:\/(.*))?$/i.exec(url.pathname);
    if (!match || ![REPO.toLowerCase(), 'zyphorah/prysma'].includes(match[1].toLowerCase())) return null;
    if (!match[2]) return null;
    return resolveRelative(path, '/' + match[2] + url.search + url.hash);
  } catch { return null; }
}
export function renderMarkdown(markdown, {path, lang, sha, pages, home, labels}) {
  const dom = DOMPurify.sanitize(marked.parse(markdown, {gfm:true, async:false}), {
    RETURN_DOM_FRAGMENT:true,
    USE_PROFILES:{html:true},
    FORBID_TAGS:['style','form','input','button','iframe','object','embed','video','audio','source','picture','link','meta'],
    FORBID_ATTR:['id','name','style','srcset','target','ping'],
  });
  if (home) {
    const first = dom.querySelector('h1');
    if (first) first.textContent = 'Prysma';
    // The small site wordmark replaces the duplicated logo in GitHub's README.
    const logo = [...dom.querySelectorAll('img')].find(img => /(?:^|\/)img\/prysma\.svg$/i.test(img.getAttribute('src') || ''));
    if (logo) {
      const container = logo.parentElement;
      if (container?.matches('p,div') && container.children.length === 1 && !container.textContent.trim()) container.remove();
      else logo.remove();
    }
  }
  for (const img of dom.querySelectorAll('img')) {
    const raw = img.getAttribute('src') || '';
    if (home && /^(?:https:\/\/img\.shields\.io\/badge\/|https:\/\/github\.com\/[^/]+\/[^/]+\/actions\/workflows\/[^/]+\/badge\.svg)/i.test(raw)) {
      const badge = img.closest('a') || img;
      const wrapper = badge.parentElement;
      badge.remove();
      if (wrapper?.matches('p') && !wrapper.textContent.trim() && !wrapper.children.length) wrapper.remove();
      continue;
    }
    const relative = repoTarget(path, raw);
    if (relative?.path && !relative.search) img.src = rawURL(sha, relative.path);
    else if (!raw.startsWith('https://raw.githubusercontent.com/' + REPO + '/')) {
      img.replaceWith(document.createTextNode(img.alt || ''));
      continue;
    }
    img.loading = 'lazy';
    img.decoding = 'async';
  }
  const known = new Map(pages.map(entry => [entry.path,entry]));
  for (const anchor of dom.querySelectorAll('a[href]')) {
    const href = anchor.getAttribute('href');
    const target = repoTarget(path, href);
    if (target) {
      const candidate = known.get(target.path);
      if (candidate && !target.search) anchor.href = routeURL(candidate.lang, candidate.path, target.section);
      else anchor.href = blobURL(sha, target.path) + target.search + (target.section ? '#' + encodeURIComponent(target.section) : '');
    } else if (!/^(?:https?:|mailto:)/i.test(href)) anchor.removeAttribute('href');
  }
  if (home) for (const block of dom.querySelectorAll('div[align="center"],p')) {
    if (!block.textContent.trim() && !block.querySelector('img,hr,code')) block.remove();
  }
  for (const quote of dom.querySelectorAll('blockquote')) {
    const paragraph = quote.firstElementChild;
    const text = paragraph?.firstChild;
    if (!paragraph?.matches('p') || text?.nodeType !== Node.TEXT_NODE) continue;
    const marker = /^\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\][ \t]*(?:\n|$)/.exec(text.data);
    if (!marker) continue;
    const type = marker[1].toLowerCase();
    text.deleteData(0,marker[0].length);
    if (paragraph.firstElementChild?.matches('br') && !text.data) paragraph.firstElementChild.remove();
    if (!paragraph.textContent.trim() && !paragraph.children.length) paragraph.remove();
    quote.classList.add('doc-alert','doc-alert-' + type);
    quote.prepend(element('p',labels.alerts[type],'alert-title'));
  }
  const headings = [];
  const seen = new Set();
  for (const heading of dom.querySelectorAll('h1,h2,h3,h4,h5,h6')) {
    const base = slug(heading.textContent) || 'section';
    let id = base;
    for (let suffix=1; seen.has(id); suffix++) id = base + '-' + suffix;
    seen.add(id);
    heading.id = 'section-' + id;
    if (heading.matches('h2,h3')) headings.push({id, text:heading.textContent, sub:heading.matches('h3')});
  }
  for (const table of dom.querySelectorAll('table')) {
    const wrap = element('div', '', 'table-scroll');
    wrap.tabIndex = 0;
    wrap.setAttribute('role', 'group');
    wrap.setAttribute('aria-label', labels.table);
    table.before(wrap);
    wrap.append(table);
  }
  for (const pre of dom.querySelectorAll('pre')) {
    const code = pre.querySelector('code');
    if (!code) continue;
    const wrap = element('div', '', 'code-box');
    const button = element('button', labels.copy, 'copy');
    button.type = 'button';
    button.addEventListener('click', async () => {
      try {
        await navigator.clipboard.writeText(code.textContent);
        button.textContent = labels.copied;
      } catch { button.textContent = labels.copyError; }
      clearTimeout(button.resetLabel);
      button.resetLabel = setTimeout(() => {button.textContent = labels.copy;}, 1900);
    });
    pre.before(wrap);
    wrap.append(pre,button);
  }
  return {dom, headings};
}
