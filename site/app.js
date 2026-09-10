"use strict";
const config = window.temperamentSite || {};
const views = {
  fifths: { file: "fifths.png", width: 1320, height: 680, alt: "Inputs around the circle with editable comma formulas, a cent-deviation bar chart and catalogue suggestions.", caption: "Set each fifth in comma fractions and calculate a chart relative to A." },
  commas: { file: "commas.png", width: 1320, height: 680, alt: "Formula list with combined comma expressions, a coloured fifth circle and a cent-deviation bar chart.", caption: "Combine Pythagorean, syntonic and schisma expressions in wide formula fields." },
  harmony: { file: "harmony.png", width: 1480, height: 900, alt: "Harmony lattice with a selected A-major chord, Principal 8 playback checkbox and adjustable reverb and loudness.", caption: "Inspect a chord's intervals and hear it on the sampled Principal 8, with adjustable reverb and loudness." }
};
for (const button of document.querySelectorAll("[data-view]")) {
  button.addEventListener("click", () => {
    const view = views[button.dataset.view];
    for (const peer of document.querySelectorAll("[data-view]")) peer.setAttribute("aria-pressed", String(peer === button));
    const img = document.getElementById("screenshot");
    img.src = "assets/" + view.file;
    img.alt = view.alt;
    img.height = view.height;
    img.width = view.width;
    for (const id of ["screenshot-link", "full-size"]) document.getElementById(id).href = img.src;
    document.getElementById("screenshot-caption").textContent = view.caption;
  });
}
for (const el of document.querySelectorAll("[data-version]")) el.textContent = config.version || "1.6.0";
const validRepository = typeof config.repository === "string" && /^[A-Za-z0-9][A-Za-z0-9-]*\/[A-Za-z0-9_.-]+$/.test(config.repository);
const repo = validRepository ? "https://github.com/" + config.repository : "";
for (const el of document.querySelectorAll("[data-repo-path]")) {
  el.href = repo ? repo + "/blob/main/" + el.dataset.repoPath : (config.localDocs || "../") + el.dataset.repoPath;
}
if (repo) {
  for (const el of document.querySelectorAll("[data-repo-suffix]")) {
    el.href = repo + el.dataset.repoSuffix;
    el.hidden = false;
  }
  if (config.hasLicense) {
    document.getElementById("license-intro").textContent = "Maintained by zurek-jiri. You may use, modify and redistribute this application under AGPLv3, including commercially.";
    const status = document.getElementById("license-status");
    status.textContent = "Provided without warranty. See the license for the terms of use and redistribution. ";
    const link = document.createElement("a");
    link.href = repo + "/blob/main/LICENSE";
    link.textContent = "Read the license →";
    status.appendChild(link);
  }
}
