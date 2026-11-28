// Optional docs/site.nav in GitHub main. Omitted pages retain their automatic category.
export function parseNavigation(text, entries) {
  const pages = new Map(entries.map(entry => [entry.path, entry]));
  const used = new Set();
  const groups = {fr: [], en: []};
  const warnings = [];
  let section;
  for (const [i, raw] of text.split(/\r?\n/).entries()) {
    const line = raw.trim();
    if (!line || line.startsWith('#')) continue;
    const bar = line.indexOf('|');
    const command = (bar < 0 ? line : line.slice(0,bar)).trim();
    const title = bar < 0 ? '' : line.slice(bar+1).trim();
    const declaration = /^section\s+(fr|en)\s+([a-z][a-z0-9-]*)$/.exec(command);
    const page = /^page\s+(.+)$/.exec(command);
    const problem = () => warnings.push(i+1);
    if (declaration) {
      const [,lang,id] = declaration;
      section = undefined;
      if (!title || groups[lang].some(group => group.id === id)) {problem(); continue;}
      section = {lang, id, title, count: 0};
      groups[lang].push(section);
    } else if (page && section) {
      const entry = pages.get(page[1]);
      if (!entry || entry.lang !== section.lang || used.has(entry.path)) {problem(); continue;}
      used.add(entry.path);
      pages.set(entry.path, {...entry, group: 'custom-' + section.id, order: ++section.count, navTitle: title});
    } else problem();
  }
  const sections = Object.fromEntries(Object.entries(groups).map(([lang,items]) =>
    [lang, items.filter(item => item.count).map(({id,title}) => ['custom-' + id,title])]));
  return {entries: [...pages.values()], sections, warnings};
}
