// sourced from https://github.com/leopatras/simple_offline_pwa (published without license)
// the copyright holder may file an issue with https://github.com/Wohlstand/TheXTech in the case of any licensing concerns

//minimalistic worker with the only goal to fulfill Chrome/Edge's PWA install requests
//no service worker caches are used, only the browser cache
//(test2.html is pre loaded in 'simple.js' via fetch API,it could be of course also loaded here)
let _isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);
console.log(">>>service worker loading,isSafari:"+_isSafari);

self.addEventListener('activate', function (event) {
  console.log("TheXTech service worker activate");
  event.waitUntil(self.clients.claim());
});

self.addEventListener('install', function(event) {
  console.log("TheXTech service worker install");
  event.waitUntil(self.skipWaiting()); // Activate worker immediately
});

self.addEventListener('fetch', function (event) {
  console.info('TheXTech service worker fetch :' + event.request.url + ",mode:" +event.request.mode);
  if (_isSafari) {
    //in case one wants to use the service worker too for Safari
    //safari works different from Chrome and FF:
    //it needs to explicitly request from network when being offline
    //otherwise loading test2.html doesn't work
    //Chrome and FF do the right thing by default
    event.respondWith(fetch(event.request));
  }
});
