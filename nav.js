(function () {
  const links = [
    { label: "Home", path: "index.htm" },
    { label: "Engineering", path: "engineering/index.htm" },
    { label: "Extras", path: "Extras/index.htm" },
    { label: "About", path: "about/index.htm" },
  ];

  function getBasePath() {
    const path = window.location.pathname;
    const marker = "/portfolio/";

    if (window.location.protocol === "file:") {
      const afterMarker = path.includes(marker)
        ? path.split(marker)[1]
        : path.replace(/^\//, "");
      const depth = Math.max(afterMarker.split("/").filter(Boolean).length - 1, 0);
      return depth ? "../".repeat(depth) : "./";
    }

    if (path.includes(marker)) {
      return marker;
    }

    return "/";
  }

  function buildNav(basePath) {
    const desktopLinks = links
      .map((link) => `\n        <span class="navlink">\n            <a href="${basePath}${link.path}">${link.label}</a>\n        </span>`)
      .join("");

    const mobileLinks = links
      .map((link) => `\n            <a href="${basePath}${link.path}">${link.label}</a>`)
      .join("");

    return `
<nav>
    <div class="navholder">${desktopLinks}
    </div>

    <div id="navholder-mobile" class="navholder-mobile">
        <a href="javascript:void(0)" class="closebtn" onclick="closeNav()">☰</a>
        <div class="navholder-content">${mobileLinks}
        </div>
    </div>

    <span class="hamburger" onclick="openNav()">☰</span>
</nav>`;
  }

  function injectNav() {
    const mount = document.getElementById("site-nav");
    if (!mount) return;
    mount.innerHTML = buildNav(getBasePath());
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", injectNav);
  } else {
    injectNav();
  }
})();
