#include "graphics/resource_font.h"

#include <iostream>

void api::graphics::ResourceFont::Setup() { LoadAllFonts(); }

sf::Font& api::graphics::ResourceFont::font(Font resource_id) {
  if (static_cast<int>(resource_id) < fonts_.size()) {
    return fonts_.at(static_cast<int>(resource_id));
  }
  return blank_font_;
}

void api::graphics::ResourceFont::LoadAllFonts() {
  blank_font_ = sf::Font();

  for (int i = 0; i < fonts_.size(); i++) {
    if (!fonts_.at(i).openFromFile(path_ + names_font_.at(i) + ".ttf")) {
      if (!fonts_.at(i).openFromFile(path_ + names_font_.at(i) + ".TTF")) {
        std::cout << "Error loading " + path_ + names_font_.at(i) + " font "
                  << std::endl;
      }
    }
  }
}
