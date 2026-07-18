"use strict";

const numberFormat = new Intl.NumberFormat("fr-CA");
const percentFormat = new Intl.NumberFormat("fr-CA", {
  minimumFractionDigits: 2,
  maximumFractionDigits: 2,
});
const dateFormat = new Intl.DateTimeFormat("fr-CA", {
  dateStyle: "medium",
  timeStyle: "medium",
});

const matchingLabels = {
  units: "Unités vérifiées",
  functions: "Fonctions byte-matchées",
  bytes: "Octets byte-matchés",
};

const analysisLabels = {
  units: "Unités analysées",
  functions: "Fonctions analysées",
  bytes: "Octets cartographiés",
};

const statusLabels = {
  working: "En travail",
  idle: "Disponible",
  blocked: "Bloqué",
  done: "Terminé",
  offline: "Hors ligne",
};

const provenanceLabels = {
  source: "Source",
  sdk: "SDK",
  fallback: "Fallback",
  unmatched: "Non apparié",
};

const loadedDashboardScript = document.currentScript;
const dashboardClientVersion = loadedDashboardScript?.src
  ? new URL(loadedDashboardScript.src, window.location.href).searchParams.get("v") || ""
  : "";

let refreshTimer;
let reloadStarted = false;

async function clearLegacyOfflineState() {
  const cleanup = [];
  if (typeof navigator !== "undefined" && navigator.serviceWorker?.getRegistrations) {
    cleanup.push(
      navigator.serviceWorker
        .getRegistrations()
        .then((registrations) => Promise.all(registrations.map((registration) => registration.unregister()))),
    );
  }
  if (typeof caches !== "undefined" && caches.keys) {
    cleanup.push(
      caches.keys().then((names) => Promise.all(names.map((name) => caches.delete(name)))),
    );
  }
  await Promise.allSettled(cleanup);
}

const offlineStateReady = clearLegacyOfflineState();

async function reloadForDashboardVersion(version) {
  if (reloadStarted) return;
  reloadStarted = true;
  clearTimeout(refreshTimer);
  await clearLegacyOfflineState();
  const target = new URL(window.location.href);
  target.searchParams.set("dashboard_version", version);
  window.location.replace(target);
}

function element(tag, className, text) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function formatCounter(counter, key = "matched") {
  if (!counter || counter.total <= 0) return "Non mesuré";
  return `${numberFormat.format(counter[key])} / ${numberFormat.format(counter.total)}`;
}

function trackMeasured(track) {
  return track.basis !== "unmeasured" &&
    ["bytes", "functions", "units"].some((name) => track.metrics[name].total > 0);
}

function formatAnalysisSummary(track) {
  for (const [name, suffix] of [
    ["bytes", "octets analysés"],
    ["functions", "fonctions analysées"],
    ["units", "unités analysées"],
  ]) {
    if (track.metrics[name].total > 0) {
      return `${formatCounter(track.metrics[name], "covered")} ${suffix}`;
    }
  }
  return "Aucune couverture d’analyse pour cette vue";
}

function renderOverallMetrics(metrics, metricLabels, counterKey) {
  const container = document.querySelector("#overall-metrics");
  container.replaceChildren();
  for (const name of ["units", "functions", "bytes"]) {
    const tile = element("div", "metric-tile");
    tile.append(element("span", "metric-label", metricLabels[name]));
    const value = element("span", "metric-value");
    if (metrics[name].total > 0) {
      value.append(document.createTextNode(numberFormat.format(metrics[name][counterKey])));
      value.append(element("span", "metric-total", ` / ${numberFormat.format(metrics[name].total)}`));
    } else {
      value.textContent = "Non mesuré";
    }
    tile.append(value);
    container.append(tile);
  }
}

function renderSection(section) {
  const card = element("article", "section-card");
  const top = element("div", "section-card-top");
  top.append(element("h3", "", section.label));
  const matchingMeasured = trackMeasured(section.matching);
  top.append(element(
    "span",
    "section-percent",
    matchingMeasured ? `${percentFormat.format(section.matching.percent)} %` : "Non mesuré",
  ));
  card.append(top);

  const track = element("progress", "progress-track");
  track.setAttribute("aria-label", `Byte-matching linké ${section.label}`);
  track.max = 100;
  track.value = Math.max(0, Math.min(100, section.matching.percent));
  track.hidden = !matchingMeasured;
  card.append(track);

  const metrics = element("dl", "mini-metrics");
  for (const name of ["units", "functions", "bytes"]) {
    const row = element("div");
    row.append(element("dt", "", matchingLabels[name]));
    row.append(element("dd", "", formatCounter(section.matching.metrics[name], "matched")));
    metrics.append(row);
  }
  card.append(metrics);

  const analysis = element("div", "analysis-detail");
  const analysisTop = element("div", "analysis-detail-top");
  const analysisMeasured = trackMeasured(section.analysis);
  analysisTop.append(element("span", "", "Couverture d’analyse"));
  analysisTop.append(
    element(
      "strong",
      "",
      analysisMeasured ? `${percentFormat.format(section.analysis.percent)} %` : "Non mesuré",
    ),
  );
  analysis.append(analysisTop);
  const analysisTrack = element("progress", "analysis-track");
  analysisTrack.max = 100;
  analysisTrack.value = Math.max(0, Math.min(100, section.analysis.percent));
  analysisTrack.hidden = !analysisMeasured;
  analysisTrack.setAttribute(
    "aria-label",
    analysisMeasured
      ? `Couverture d’analyse ${section.label}`
      : `Couverture d’analyse ${section.label} non mesurée`,
  );
  analysis.append(analysisTrack);
  analysis.append(
    element(
      "span",
      "analysis-count",
      formatAnalysisSummary(section.analysis),
    ),
  );
  card.append(analysis);
  return card;
}

function renderSections(sections) {
  const binaries = document.querySelector("#binary-sections");
  const categories = document.querySelector("#category-sections");
  binaries.replaceChildren();
  categories.replaceChildren();

  for (const section of sections.filter((item) => item.dimension === "section")) {
    binaries.append(renderSection(section));
  }
  const availableCategories = sections.filter(
    (item) => item.dimension === "category" && item.available,
  );
  for (const section of availableCategories) categories.append(renderSection(section));
  document.querySelector("#categories-panel").hidden = availableCategories.length === 0;
}

function renderWorkers(workers) {
  const container = document.querySelector("#workers");
  container.replaceChildren();
  if (!workers.length) {
    container.append(element("p", "empty-state", "Aucun worker configuré."));
    return;
  }
  for (const worker of workers) {
    const row = element("div", "worker");
    const status = element("span", `worker-status ${worker.status}`);
    status.title = statusLabels[worker.status] || worker.status;
    row.append(status);

    const copy = element("div");
    copy.append(element("div", "worker-name", worker.label));
    const suffix = worker.stale && worker.reported_status !== "offline" ? " · heartbeat expiré" : "";
    copy.append(
      element("div", "worker-task", `${statusLabels[worker.status] || worker.status}${suffix} — ${worker.task || "Aucune tâche déclarée"}`),
    );
    row.append(copy);
    if (worker.section) row.append(element("span", "worker-section", worker.section));
    row.title = worker.updated_at
      ? `Dernière mise à jour : ${dateFormat.format(new Date(worker.updated_at))}`
      : "Aucun heartbeat";
    container.append(row);
  }
}

function renderChanges(changes) {
  const container = document.querySelector("#changes");
  container.replaceChildren();
  if (!changes.length) {
    container.append(element("li", "empty-state", "Aucun changement enregistré pour le moment."));
    return;
  }
  for (const change of changes) {
    const item = element("li");
    item.append(element("span", "change-summary", change.summary));
    const details = [];
    if (change.section) details.push(change.section);
    if (change.track) details.push(change.track === "matching" ? "byte-matching" : "analyse");
    if (change.provenance) details.push(provenanceLabels[change.provenance] || change.provenance);
    if (change.track === "matching" && change.matching_credited === false) {
      details.push("preuve non linkée · match non crédité");
    }
    if (change.worker) details.push(change.worker);
    details.push(dateFormat.format(new Date(change.updated_at)));
    item.append(element("span", "change-meta", details.join(" · ")));
    container.append(item);
  }
}

function renderPublication(data) {
  const build = data.build || {};
  const provenance = data.overall.provenance || {};
  const units = data.overall.pipeline_units || {};
  const lastError = build.last_error;
  const publishedLabel = build.published_at
    ? dateFormat.format(new Date(build.published_at))
    : "Vue live non publiée";
  const shortCommit = build.commit ? build.commit.slice(0, 8) : "inconnu";

  document.querySelector("#build-id").textContent = build.id || "Non publié";
  document.querySelector("#build-commit").textContent = build.commit || "Inconnu";
  document.querySelector("#published-at").textContent = publishedLabel;
  document.querySelector("#hero-build-id").textContent = build.id || "Non publié";
  document.querySelector("#hero-build-meta").textContent =
    `commit ${shortCommit} · ${publishedLabel}`;
  document.querySelector("#provenance-counts").textContent = ["source", "sdk", "fallback", "unmatched"]
    .map((name) => `${provenanceLabels[name]} ${numberFormat.format(provenance[name] || 0)}`)
    .join(" · ");
  document.querySelector("#pipeline-units").textContent =
    `${numberFormat.format(units.probe || 0)} probe · ` +
    `${numberFormat.format(units.promoted || 0)} promues · ` +
    `${numberFormat.format(units.rejected || 0)} rejetées`;

  const error = document.querySelector("#last-publication-error");
  error.hidden = !lastError;
  error.textContent = lastError
    ? `Dernière erreur : ${typeof lastError === "string" ? lastError : lastError.message}`
    : "";
}

function renderIssues(issues) {
  const panel = document.querySelector("#issues-panel");
  const list = document.querySelector("#issues");
  list.replaceChildren();
  for (const issue of issues) list.append(element("li", "", issue));
  panel.hidden = issues.length === 0;
}

function render(data) {
  document.title =
    `${data.project.short_name} — Byte-match ${percentFormat.format(data.overall.matching.percent)} %` +
    ` · analyse ${percentFormat.format(data.overall.analysis.percent)} %`;
  document.querySelector("#project-name").textContent = data.project.name;
  document.querySelector("#overall-percent").textContent = percentFormat.format(data.overall.matching.percent);
  document.querySelector("#updated-at").textContent =
    `Recalculé ${dateFormat.format(new Date(data.generated_at))} · ` +
    `${data.overall.total_sections} exécutables · baseline ${numberFormat.format(data.overall.analysis.metrics.bytes.total)} octets`;
  document.querySelector("#analysis-percent").textContent =
    `${percentFormat.format(data.overall.analysis.percent)} %`;
  document.querySelector("#analysis-progress").value = data.overall.analysis.percent;
  document.querySelector("#analysis-bytes").textContent =
    `${formatCounter(data.overall.analysis.metrics.bytes, "covered")} octets analysés`;
  document.querySelector("#schema-version").textContent = data.schema_version;
  document.querySelector("#source-counts").textContent =
    `${numberFormat.format(data.sources.evidence)} preuves · ` +
    `${numberFormat.format(data.sources.matching_linked || 0)} linkées · ` +
    `${numberFormat.format(data.sources.matching_rejected || 0)} rejetées · ` +
    `${numberFormat.format(data.sources.workers)} workers · ` +
    `${numberFormat.format(data.sources.changes)} changements`;
  renderOverallMetrics(data.overall.matching.metrics, matchingLabels, "matched");
  renderSections(data.sections);
  renderWorkers(data.workers);
  renderChanges(data.recent_changes);
  renderPublication(data);
  renderIssues(data.issues);
}

function setConnection(state, label) {
  const live = document.querySelector("#live-state");
  live.className = `live-state ${state}`;
  document.querySelector("#live-label").textContent = label;
}

async function refresh() {
  clearTimeout(refreshTimer);
  let delay = 5000;
  try {
    await offlineStateReady;
    const clientVersion = encodeURIComponent(dashboardClientVersion || "legacy");
    const response = await fetch(
      `/api/progress?refresh=${Date.now()}&client_version=${clientVersion}`,
      { cache: "no-store" },
    );
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const data = await response.json();
    if (data.error) throw new Error(data.error);
    if (data.dashboard_version && data.dashboard_version !== dashboardClientVersion) {
      await reloadForDashboardVersion(data.dashboard_version);
      return;
    }
    render(data);
    setConnection(data.issues.length ? "error" : "connected", data.issues.length ? "Données partielles" : "Recalcul en direct");
    delay = Math.max(1000, data.refresh_seconds * 1000);
  } catch (error) {
    setConnection("error", "Serveur indisponible");
    document.querySelector("#updated-at").textContent = `Échec de mise à jour : ${error.message}`;
  }
  if (!reloadStarted) refreshTimer = setTimeout(refresh, delay);
}

refresh();
