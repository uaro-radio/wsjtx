/* Progressive enhancement: replace the static "latest release" labels with the
   current Latest Release from GitHub. If the API is unavailable (offline, rate
   limit, error), the static text in the HTML is left untouched. */
(function () {
  var vEls = document.querySelectorAll(".js-latest-version");
  var dEls = document.querySelectorAll(".js-latest-date");
  if (!vEls.length && !dEls.length) return;

  fetch("https://api.github.com/repos/WSJTX/wsjtx/releases/latest", {
    headers: { Accept: "application/vnd.github+json" }
  })
    .then(function (r) { return r.ok ? r.json() : null; })
    .then(function (rel) {
      if (!rel || !rel.tag_name) return;
      var num = String(rel.tag_name).replace(/^v/, "");
      var name = rel.name && /WSJT-X/i.test(rel.name) ? rel.name.trim() : "WSJT-X " + num;
      vEls.forEach(function (el) { el.textContent = name; });
      if (rel.published_at) {
        var d = new Date(rel.published_at);
        var s = d.toLocaleDateString("en-US", { day: "numeric", month: "short", year: "numeric" });
        dEls.forEach(function (el) { el.textContent = s; });
      }
    })
    .catch(function () { /* keep the static fallback */ });
})();
