//
//  history.cpp
//  MiniBash
//
//  Created by Chigozie Oduah on 05/09/2025.
//
#include <string>

#include "history.hpp"
#include "configs.hpp"

History& History::getInstance() {
    static History instance;
    return instance;
}

std::string History::get_current() {
    return items[position];
}

bool History::move_up() {
    if (position > 0) {
        position--;
        return true;
    }
    return false;
}

bool History::move_down() {
    if (items.size() > 0 && position < items.size() - 1) {
        position++;
        return true;
    }
    return false;
}

void History::add_item(std::string new_item) {
    Configs& configs = Configs::getInstance();
    
    items.push_back(new_item);
    while (items.size() > configs.history_max_size) {
        items.pop_front();
    }
    position = items.size();
}
