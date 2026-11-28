import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
import {existsSync, mkdtempSync, readFileSync, rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import test from 'node:test';
import {fileURLToPath} from 'node:url';
import {JSDOM} from 'jsdom';

test('published docs are complete HTML with crawlable local links', t => {
  const output = mkdtempSync(join(tmpdir(),'prysma-website-'));
  t.after(() => rmSync(output,{recursive:true,force:true}));
  execFileSync(process.execPath,[fileURLToPath(new URL('../build.mjs', import.meta.url)),output]);
  const routes = JSON.parse(readFileSync(join(output,'routes.json'),'utf8'));
  const base = 'https://prysma-llvm.github.io/prysma/';
  const sitemap = new JSDOM(readFileSync(join(output,'sitemap.xml'),'utf8'),{contentType:'text/xml'});
  assert.deepEqual(new Set([...sitemap.window.document.querySelectorAll('loc')].map(node => node.textContent)),
    new Set(Object.values(routes).map(route => new URL(route,base).href)));
  sitemap.window.close();
  for (const route of Object.values(routes)) {
    const canonical = new URL(route,base);
    const doc = new JSDOM(readFileSync(join(output, decodeURIComponent(route), 'index.html'),'utf8'),{url:canonical.href});
    const d = doc.window.document;
    assert.equal(d.querySelector('link[rel="canonical"]').href,canonical.href);
    assert.equal(d.querySelectorAll('main article h1').length,1,route);
    assert.ok(d.querySelector('article').textContent.trim().length > 100,route);
    for (const asset of d.querySelectorAll('link[rel="stylesheet"], script[src], #nav a[href], link[rel="alternate"]')) {
      const target = new URL(asset.href || asset.src);
      assert.ok(target.href.startsWith(base),target.href);
      const path = decodeURIComponent(target.pathname.slice(new URL(base).pathname.length));
      const file = path.endsWith('/') || !path ? path + 'index.html' : path;
      assert.ok(existsSync(join(output,file)),target.href);
    }
    if (!route) {
      assert.ok(d.querySelector('meta[name="google-site-verification"]'));
      assert.match(d.querySelector('article').textContent,/LLVM 18/);
    }
    if (route.endsWith('/first_program/')) {
      assert.match(d.querySelector('article pre code').textContent,/git clone/);
      assert.equal(d.querySelectorAll('link[rel="alternate"]').length,2);
    }
    doc.window.close();
  }
});
