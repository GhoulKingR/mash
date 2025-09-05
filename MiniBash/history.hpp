//
//  history.hpp
//  MiniBash
//
//  Created by Chigozie Oduah on 05/09/2025.
//
#ifndef HISTORY_HPP
#define HISTORY_HPP

#include <deque>

class History {
    std::deque<std::string> items;
    size_t position = 0;
    
    History() = default;
    ~History() = default;
    
public:
    History(const History&) = delete;
    History& operator=(const History&) = delete;
    History(History&&) = delete;
    History& operator=(History&&) = delete;
    
    std::string get_current();
    bool move_up();
    bool move_down();
    void add_item(std::string new_item);
    
    static History& getInstance();
};

#endif
