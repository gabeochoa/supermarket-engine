
#ifndef UTIL_H
#define UTIL_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>

// string split
#include <regex>
std::vector<std::string> split(const std::string& input,
                               const std::string& regex) {
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(regex);
    std::sregex_token_iterator first{input.begin(), input.end(), re, -1};
    std::sregex_token_iterator last;
    return std::vector<std::string>{first, last};
}

// profileing stuff
#include <chrono>
#include <deque>
typedef std::deque<double> Samples;
std::map<std::string, Samples> _acc;
struct prof {
    std::string name;
    std::chrono::system_clock::time_point tstart;

    prof(std::string n) : name(n) { tstart = std::chrono::system_clock::now(); }
    ~prof() {
        std::chrono::system_clock::time_point tend =
            std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(tend -
                                                                  tstart);
        std::map<std::string, Samples>::iterator p = _acc.find(name);
        if (p == _acc.end()) {
            _acc.insert(std::make_pair(name, std::deque<double>()));
        }

        _acc[name].push_back(elapsed.count());

        if (_acc[name].size() > 1000) {
            _acc[name].pop_front();
        }
    }
};

// https://stackoverflow.com/a/29856690
const std::string computeMethodName(const std::string& function,
                                    const std::string& prettyFunction) {
    size_t locFunName = prettyFunction.find(
        function);  // If the input is a constructor, it gets the beginning of
                    // the class name, not of the method. That's why later on we
                    // have to search for the first parenthesys
    size_t begin = prettyFunction.rfind(" ", locFunName) + 1;
    size_t end = prettyFunction.find(
        "(", locFunName + function.length());  // Adding function.length() make
                                               // this faster and also allows to
                                               // handle operator parenthesis!
    if (prettyFunction[end + 1] == ')')
        return (prettyFunction.substr(begin, end - begin) + "()");
    else
        return (prettyFunction.substr(begin, end - begin) + "(...)");
}

#define __PROFILE_FUNC__ computeMethodName(__FUNCTION__, __PRETTY_FUNCTION__)

// end profiling stuff

constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;

const char* debug_env = std::getenv("DEBUG");
bool DEBUG = debug_env != nullptr;

typedef sf::Vector2f vec2f;
typedef sf::Vector2<bool> vec2b;
typedef sf::Vector2<int> vec2i;

struct World;

bool DEBUG_show_colliders = false;
sf::Font* FONTPTR = nullptr;
World* world = nullptr;
sf::RenderWindow* window = nullptr;
sf::View* game_view = nullptr;

std::map<std::string, sf::Keyboard::Key> KEYS;

void default_keys() {
    KEYS["1"] = sf::Keyboard::Num1;
    KEYS["2"] = sf::Keyboard::Num2;
    KEYS["Left"] = sf::Keyboard::A;
    KEYS["Right"] = sf::Keyboard::D;
    KEYS["Up"] = sf::Keyboard::W;
    KEYS["Down"] = sf::Keyboard::S;
    KEYS["Scroll-In"] = sf::Keyboard::PageDown;
    KEYS["Scroll-Out"] = sf::Keyboard::PageUp;
}

void load_keys() {
    // load default keys
    default_keys();

    // load keybindings from file
    std::ifstream ifs("./resources/keybindings.ini");
    if (!ifs.is_open()) {
        // TODO add error message
        return;
    }
    std::string line;
    while (getline(ifs, line)) {
        auto tokens = split(line, ",");
        KEYS[tokens[0]] = static_cast<sf::Keyboard::Key>(std::stoi(tokens[1]));
    }
    ifs.close();
}

void export_keys() {
    std::ofstream ofs("./resources/keybindings.ini");
    if (!ofs.is_open()) {
        // TODO add error message
        return;
    }

    for (auto const& kv : KEYS) {
        ofs << kv.first << "," << kv.second << std::endl;
    }

    ofs.close();
}

void setGlobalFont(sf::Font* f) { FONTPTR = f; }

sf::Font* getGlobalFont() { return FONTPTR; }

void setGlobalWorld(World* w) { world = w; }
World* getGlobalWorld() { return world; }

void setGlobalWindow(sf::RenderWindow* w) { window = w; }
sf::RenderWindow* getGlobalWindow() { return window; }

float flerp(float a, float b, float t) { return a + t * (b - a); }

float dtr(float deg) { return deg * (M_PI / 180); }

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void log(std::string line, bool nl = true) {
    std::cout << line;
    if (nl) std::cout << std::endl;
}

std::ostream& operator<<(std::ostream& os, vec2f const& v) {
    return os << "(" << v.x << "," << v.y << ")";
}

std::ostream& operator<<(std::ostream& os, vec2i const& v) {
    return os << "(" << v.x << "," << v.y << ")";
}

template <>
struct fmt::formatter<vec2f> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(vec2f const& v, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "({},{})", v.x, v.y);
    }
};

template <>
struct fmt::formatter<vec2i> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(vec2i const& v, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "({},{})", v.x, v.y);
    }
};

std::ostream& operator<<(std::ostream& os, std::vector<vec2f> const& vf) {
    os << "vector<v2f>: [";
    os << std::endl;
    for (const vec2f& v : vf) {
        os << "  " << v << std::endl;
    }
    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, std::vector<vec2i> const& vi) {
    os << "vector<v2i>: [";
    os << std::endl;
    for (const vec2i& v : vi) {
        os << "  " << v << std::endl;
    }
    os << "]";
    return os;
}

vec2f min(vec2f a, vec2f b) { return vec2f{fmin(a.x, b.x), fmin(a.y, b.y)}; }

vec2f max(vec2f a, vec2f b) { return vec2f{fmax(a.x, b.x), fmax(a.y, b.y)}; }

vec2f clamp(vec2f a, vec2f mx, vec2f mn) { return max(min(a, mx), mn); }

float mag(vec2f a) { return sqrt(a.x * a.x + a.y * a.y); }

vec2f mul(vec2f a, float b) { return vec2f{a.x * b, a.y * b}; }

float dist(vec2f a, vec2f b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

float dist_i(vec2i a, vec2i b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

vec2i vtoi(vec2f a) { return vec2i{(int)a.x, (int)a.y}; }
vec2f vtof(vec2i a) { return vec2f{1.0f * a.x, 1.0f * a.y}; }

vec2f norm(vec2f a) {
    float m = mag(a);
    if (m == 0.f) {
        return vec2f{0.f, 0.f};
    }
    return mul(a, abs(1 / m));
}

float dot(vec2f a, vec2f b) { return (a.x * b.x) + (a.y * b.y); }

bool isNearlyEqual(double a, double b) {
    double factor = 0.000001;  // epsilon

    double min_a =
        a -
        (a - std::nextafter(a, std::numeric_limits<double>::lowest())) * factor;
    double max_a =
        a +
        (std::nextafter(a, std::numeric_limits<double>::max()) - a) * factor;

    return min_a <= b && max_a >= b;
}

sf::FloatRect boundingRect(const sf::Shape& shape,
                           float outlineThickness = 0.f) {
    // Is this case handled or left as UB?
    if (shape.getPointCount() == 0) return sf::FloatRect();

    sf::Vector2f min = shape.getTransform().transformPoint(shape.getPoint(0));
    sf::Vector2f max = min;

    for (std::size_t i = 1; i < shape.getPointCount(); ++i) {
        sf::Vector2f point =
            shape.getTransform().transformPoint(shape.getPoint(i));

        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
    }

    // take into account outline
    min.x -= outlineThickness;
    min.y -= outlineThickness;
    max.x += outlineThickness;
    max.y += outlineThickness;

    return sf::FloatRect(min, max - min);
}

sf::Text getText(const std::string t, int size = 48,
                 sf::Color color = sf::Color::Red,
                 sf::Text::Style style = sf::Text::Regular) {
    sf::Text out;
    out.setFont(*getGlobalFont());
    out.setString(t);
    out.setCharacterSize(size);
    out.setFillColor(color);
    out.setStyle(style);
    return out;
}

// from https://gist.github.com/cbsmith/5538174
// use by doing :
//
//  random_selector<> selector{};
//  yield selector(container)
//
#include <iterator>
#include <random>

int randIn(int a, int b) { return a + (std::rand() % (b - a + 1)); }
vec2i randVeci(int a, int b) { return vec2i{randIn(a, b), randIn(a, b)}; }
vec2f randVecf(int a, int b) { return vtof(vec2i{randIn(a, b), randIn(a, b)}); }

template <typename RandomGenerator = std::default_random_engine>
struct random_selector {
    // On most platforms, you probably want to use
    // std::random_device("/dev/urandom")()
    random_selector(RandomGenerator g = RandomGenerator(std::random_device()()))
        : gen(g) {}

    template <typename Iter>
    Iter select(Iter start, Iter end) {
        std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
        std::advance(start, dis(gen));
        return start;
    }

    // convenience function
    template <typename Iter>
    Iter operator()(Iter start, Iter end) {
        return select(start, end);
    }

    // convenience function that works on anything with a sensible begin() and
    // end(), and returns with a ref to the value type
    template <typename Container>
    auto operator()(const Container& c) -> decltype(*begin(c))& {
        return *select(begin(c), end(c));
    }

   private:
    RandomGenerator gen;
};

#endif
