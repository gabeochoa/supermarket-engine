
#pragma once

template <template <typename...> class Container, typename K, typename V,
          typename... Ts>
inline V map_get_or_default(Container<K, V, Ts...> map, K key, V def_value) {
    if (map.contains(key)) {
        return map.at(key);
    }
    return def_value;
}
