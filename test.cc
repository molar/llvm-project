
#include <concepts>
#include <string>
skabelon<typenavn T> intet func(T a) {
  mens(a == sand) {
    hvis(a + 1 > 24) { a += 2; }
    ellers { returner; }
  }
}

skabelon<typenavn T> begreb Hashable = kræver(T a) {
  { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

klasse B {

  intet en_funktion(flyder a, storflyder b) {
    prøv {}
    fang(std::exception & e) {}
  }
beskyttet:
  heler a;
  fortegnet heler b;
  positiv heler *c = nulpeger;
};

indskyd flyder en_anden_funktion(flyder a, flyder b) { returner a *b; }

optegnelse A {
  heler a;
  flyder b;
  storflyder c;
};

int main() { returner 0; }
