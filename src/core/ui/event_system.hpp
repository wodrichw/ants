#pragma once

#include <SDL2/SDL_events.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "subscriber.hpp"

using ulong = unsigned long;

template <typename Enum, typename Event>
struct EventPublisher {
    void add(Enum key, Subscriber<Event>* subscriber) {
        get(key)->push_back(subscriber);
    }

    void remove(Enum e, Subscriber<Event>* subscriber) {
        ulong key = std::hash(e);
        auto it = subscriber_map.find(key);
        if(it == subscriber_map.end()) {
            return;
        }

        auto& subscribers = *it->second;
        subscribers.erase(
            std::remove(subscribers.begin(), subscribers.end(), subscriber),
            subscribers.end());
    }

    void notify(Event const& event) {
        std::hash<Event> hash;
        ulong key = hash(event);
        auto it = subscriber_map.find(key);
        if(it == subscriber_map.end()) {
            return;
        }
        for(auto& subscriber : *it->second) {
            (*subscriber)(event);
        }
    }

    ~EventPublisher() {
        for(auto& pair : subscriber_map) {
            delete pair.second;
        }
    }

   private:
    std::vector<Subscriber<Event>*>* get(Enum e) {
        std::hash<Enum> hash;
        ulong key = hash(e);
        auto it = subscriber_map.find(key);
        if(it != subscriber_map.end()) {
            return (*it).second;
        }
        return subscriber_map[key] = new std::vector<Subscriber<Event>*>();
    }

    std::unordered_map<ulong, std::vector<Subscriber<Event>*>*> subscriber_map = {};
};

struct EventSystem {
    EventPublisher<MouseEventType, MouseEvent> mouse_events;
    EventPublisher<KeyboardEventType, KeyboardEvent> keyboard_events;
    EventPublisher<KeyboardChordEventType, KeyboardChordEvent> keyboard_chord_events;
    EventPublisher<CharKeyboardEventType, CharKeyboardEvent> char_keyboard_events;
};

void handle_quit_event();

MouseEventType get_mouse_type(char button);
void set_mouse_type(SDL_MouseButtonEvent const& event,
                     MouseEvent& mouse_event);

KeyboardEventType get_keyboard_type(SDL_Keycode event);
void set_keyboard_type(SDL_Keysym const& event, KeyboardEvent& keyboard_event);
void set_keyboard_chord_type(SDL_Keysym const& event, KeyboardChordEvent& keyboard_event);
void unset_keyboard_chord_type(SDL_Keysym const& event, KeyboardChordEvent& keyboard_event);

SDL_Keycode get_keyboard_key(SDL_Keysym const& event);
void set_char_keyboard_type(SDL_Keysym const& event,
                             CharKeyboardEvent& char_keyboard_event);
