import test from 'node:test';
import assert from 'node:assert/strict';
import {parseNavigation} from '../assets/navigation.mjs';
import {classify, makeIndex, rawURL, loadMain, resolveRelative, routeURL, readRoute, slug, fold, HOME} from '../assets/source.mjs';

const sha = 'a'.repeat(40);
const head = {sha,commit:{tree:{sha:'b'.repeat(40)}}};
const response = value => new Response(JSON.stringify(value));
const blob = path => ({path,type:'blob',mode:'100644'});

test('language pairs, groups and scope are detected from paths',() => {
  assert.equal(classify('docs/fr/language_specification.md').key,classify('docs/en/prysma.md').key);
  assert.equal(classify('docs/fr/robustness_security.md').key,classify('docs/en/robustness.md').key);
  assert.deepEqual(classify('tests/environnement_dev_tests/README.FR.md'),{
    lang:'fr',key:'tests/environnement_dev_tests/README.md',group:'operations'});
  assert.equal(classify('docs/en/installation/dashboard/guide.md').group,'operations');
  assert.equal(classify('docs/fr/llvm_documentation/decompilation/array.c'),null);
  assert.equal(classify('website/README.md'),null);
});
test('routes preserve Unicode, spaces and the anchored heading',() => {
  const path='docs/fr/installation/à lire.md', section='étape-1';
  assert.deepEqual(readRoute(routeURL('fr',path,section)),{path,lang:'fr',section});
  assert.equal(readRoute('', 'fr').path, HOME.fr);
  assert.equal(readRoute('#fr/%','en').path, HOME.en);
  assert.deepEqual(resolveRelative('docs/fr/install/a.md','../guide.md#%C3%A9tape'),{
    path:'docs/fr/guide.md',search:'',section:'étape'});
  assert.equal(resolveRelative('README.md','javascript:alert(1)'),null);
  assert.equal(slug('Déjà vu : LLVM 18'), 'déjà-vu--llvm-18');
  assert.equal(fold('Déjà vu'), 'deja vu');
});
test('unchanged main takes one request, changed main discovers new Markdown',async () => {
  const calls=[];
  const known=makeIndex(sha,Object.values(HOME));
  const fetcher=async url => {calls.push(url);return response(head);};
  assert.equal(await loadMain(known,{fetcher}),known);
  assert.equal(calls.length,1);
  const next=await loadMain(null,{fetcher:async url => url.includes('/commits/')
    ? response(head) : response({truncated:false,tree:[...Object.values(HOME).map(blob),blob('docs/en/new_page.md'),
        {...blob('docs/en/symlink.md'),mode:'120000'}]})});
  assert.equal(next.sha,sha);
  assert.equal(next.entries.find(p=>p.path.endsWith('new_page.md')).title,'New page');
  assert.ok(!next.entries.some(p=>p.path.endsWith('symlink.md')));
});
test('unavailable and partial GitHub data do not silently replace the index',async ()=>{
  await assert.rejects(loadMain(null,{fetcher:async()=>new Response('',{status:403})}),/403/);
  await assert.rejects(loadMain(null,{fetcher:async url=>response(url.includes('/commits/')?head:{truncated:true,tree:[]})}),/Incomplete/);
});

test('optional remote navigation adds order and labels without excluding pages', async () => {
  const path='docs/fr/language_specification.md';
  const entries=makeIndex(sha,[...Object.values(HOME),path]).entries;
  const nav='section fr debut | Prise en main\npage '+path+' | Référence\npage README.FR.md\npage README.md\npage '+path+'\npage missing.md';
  const result=parseNavigation(nav,entries);
  assert.deepEqual(result.sections.fr,[['custom-debut','Prise en main']]);
  assert.deepEqual(result.warnings,[4,5,6]);
  assert.equal(result.entries.find(p=>p.path===path).navTitle,'Référence');
  assert.equal(result.entries.find(p=>p.path===HOME.fr).order,2);
  assert.equal(result.entries.find(p=>p.path===HOME.en).group,'start');
  const live=await loadMain(null,{fetcher:async url => url.includes('/commits/')
    ? response(head) : url.includes('/git/trees/')
      ? response({truncated:false,tree:[...Object.values(HOME),path,'docs/site.nav'].map(blob)})
      : (assert.equal(url,rawURL(sha,'docs/site.nav')),new Response(nav))});
  assert.deepEqual(live.sections,result.sections);
  assert.deepEqual(live.entries,result.entries);
});
test('if navigation temporarily fails, discovery still works and retries the same revision',async()=>{
  const commits=async url=> url.includes('/commits/') ? response(head) : url.includes('/git/trees/')
    ? response({truncated:false,tree:[...Object.values(HOME),'docs/site.nav'].map(blob)})
    : new Response('',{status:503});
  const index=await loadMain(null,{fetcher:commits});
  assert.equal(index.navUnavailable,true);
  const next=await loadMain(index,{fetcher:async url => url.endsWith('/docs/site.nav')
    ? new Response('section en start | Intro\npage README.md') : commits(url)});
  assert.equal(next.navUnavailable,undefined);
  assert.deepEqual(next.sections.en,[['custom-start','Intro']]);
});
