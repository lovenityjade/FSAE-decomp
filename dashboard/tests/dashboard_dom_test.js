"use strict";

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

class FakeNode {
  constructor(tagName = "div") {
    this.tagName = tagName;
    this.children = [];
    this.attributes = {};
    this.className = "";
    this.textContent = "";
    this.hidden = false;
    this.value = 0;
    this.max = 0;
    this.title = "";
  }

  append(...children) {
    this.children.push(...children);
  }

  replaceChildren(...children) {
    this.children = [...children];
  }

  setAttribute(name, value) {
    this.attributes[name] = String(value);
  }
}

const ids = [
  "overall-metrics",
  "binary-sections",
  "category-sections",
  "categories-panel",
  "workers",
  "changes",
  "issues-panel",
  "issues",
  "project-name",
  "overall-percent",
  "updated-at",
  "analysis-percent",
  "analysis-progress",
  "analysis-bytes",
  "schema-version",
  "source-counts",
  "build-id",
  "build-commit",
  "published-at",
  "provenance-counts",
  "pipeline-units",
  "last-publication-error",
  "live-state",
  "live-label",
];
const nodes = Object.fromEntries(ids.map((id) => [id, new FakeNode()]));
const document = {
  title: "",
  createElement(tagName) {
    return new FakeNode(tagName);
  },
  createTextNode(text) {
    const node = new FakeNode("#text");
    node.textContent = text;
    return node;
  },
  querySelector(selector) {
    assert.match(selector, /^#[a-z-]+$/);
    const node = nodes[selector.slice(1)];
    assert.ok(node, `missing fake DOM node ${selector}`);
    return node;
  },
};

const dashboardRoot = path.resolve(__dirname, "..");
const source = fs.readFileSync(path.join(dashboardRoot, "app.js"), "utf8");
const context = vm.createContext({
  clearTimeout() {},
  console,
  Date,
  document,
  Error,
  fetch() {
    return new Promise(() => {});
  },
  Intl,
  Math,
  Promise,
  setTimeout() {
    return 1;
  },
});
vm.runInContext(source, context, { filename: "dashboard/app.js" });

const metric = (covered, matched, total) => ({ covered, matched, total });
const analysisMetrics = {
  units: metric(3534, 0, 3534),
  functions: metric(7315, 0, 10923),
  bytes: metric(1169561, 0, 1666308),
};
const matchingMetrics = {
  units: metric(0, 2, 3),
  functions: metric(0, 0, 0),
  bytes: metric(0, 454584, 1666308),
};
const data = {
  schema_version: 2,
  generated_at: "2026-07-17T22:30:56Z",
  build: {
    id: "validated-block-7",
    commit: "abcdef1234567890",
    published_at: "2026-07-17T22:31:00Z",
    last_error: null,
  },
  project: { name: "Four Swords Anniversary Edition", short_name: "FSAE" },
  overall: {
    total_sections: 4,
    percent: 70.19,
    metrics: analysisMetrics,
    matching: { percent: 27.28, metrics: matchingMetrics },
    analysis: { percent: 70.19, metrics: analysisMetrics },
    provenance: { source: 5, sdk: 2, fallback: 1, unmatched: 1 },
    pipeline_units: { probe: 14, promoted: 7, rejected: 3 },
  },
  sections: [
    {
      id: "arm9",
      label: "ARM9",
      dimension: "section",
      available: true,
      percent: 58.38,
      matching: {
        percent: 0,
        metrics: {
          units: metric(0, 0, 0),
          functions: metric(0, 0, 0),
          bytes: metric(0, 0, 1193472),
        },
      },
      analysis: {
        percent: 58.38,
        metrics: {
          units: metric(992, 0, 992),
          functions: metric(4646, 0, 8254),
          bytes: metric(696726, 0, 1193472),
        },
      },
    },
    {
      id: "data",
      label: "Data",
      dimension: "category",
      available: true,
      percent: 100,
      matching: {
        percent: 0,
        basis: "unmeasured",
        metrics: {
          units: metric(0, 0, 0),
          functions: metric(0, 0, 0),
          bytes: metric(0, 0, 0),
        },
      },
      analysis: {
        percent: 100,
        basis: "units",
        metrics: {
          units: metric(29, 0, 29),
          functions: metric(0, 0, 0),
          bytes: metric(0, 0, 0),
        },
      },
    },
  ],
  workers: [],
  recent_changes: [],
  issues: [],
  sources: {
    evidence: 9,
    matching_linked: 2,
    matching_rejected: 1,
    workers: 4,
    changes: 14,
  },
};

context.render(data);

assert.equal(nodes["overall-percent"].textContent, "27,28");
assert.equal(nodes["analysis-percent"].textContent, "70,19 %");
assert.equal(nodes["analysis-progress"].value, 70.19);
assert.match(document.title, /Byte-match 27,28 %/);
assert.match(document.title, /analyse 70,19 %/);

assert.equal(nodes["overall-metrics"].children.length, 3);
assert.equal(nodes["overall-metrics"].children[0].children[0].textContent, "Unités vérifiées");
assert.equal(
  nodes["overall-metrics"].children[2].children[1].children[0].textContent.replace(/\D/g, ""),
  "454584",
);

assert.equal(nodes["binary-sections"].children.length, 1);
const arm9 = nodes["binary-sections"].children[0];
assert.equal(arm9.children[0].children[1].textContent, "0,00 %");
assert.equal(arm9.children[1].value, 0);
assert.equal(arm9.children[3].className, "analysis-detail");
assert.equal(arm9.children[3].children[0].children[0].textContent, "Couverture d’analyse");
assert.equal(arm9.children[3].children[0].children[1].textContent, "58,38 %");

assert.equal(nodes["category-sections"].children.length, 1);
const dataCard = nodes["category-sections"].children[0];
assert.equal(dataCard.children[0].children[1].textContent, "Non mesuré");
assert.equal(dataCard.children[1].hidden, true);
assert.equal(dataCard.children[3].children[0].children[1].textContent, "100,00 %");
assert.equal(dataCard.children[3].children[1].hidden, false);
assert.doesNotMatch(JSON.stringify(dataCard), /0 \/ 0/);

assert.equal(nodes["build-id"].textContent, "validated-block-7");
assert.equal(nodes["build-commit"].textContent, "abcdef1234567890");
assert.match(nodes["published-at"].textContent, /2026/);
assert.equal(nodes["provenance-counts"].textContent, "Source 5 · SDK 2 · Fallback 1 · Non apparié 1");
assert.equal(nodes["pipeline-units"].textContent, "14 probe · 7 promues · 3 rejetées");
assert.equal(nodes["last-publication-error"].hidden, true);
assert.match(nodes["source-counts"].textContent, /2 linkées · 1 rejetées/);

data.build.last_error = { message: "duplicate id matching-arm9" };
context.render(data);
assert.equal(nodes["last-publication-error"].hidden, false);
assert.equal(
  nodes["last-publication-error"].textContent,
  "Dernière erreur : duplicate id matching-arm9",
);

const html = fs.readFileSync(path.join(dashboardRoot, "index.html"), "utf8");
assert.match(html, /Reconstruction byte-matching vérifiée/);
assert.match(html, /preuve de linkage exacte/);
assert.match(html, /id="build-id"/);
assert.match(html, /id="last-publication-error"/);
assert.doesNotMatch(html, />0 \/ 0 octet byte-matché</);
assert.doesNotMatch(html, /Décompilation byte-matching/);

console.log("dashboard DOM hierarchy: linked matching primary, analysis and publication visible");
