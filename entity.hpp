#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>

enum DrawLayer {
  BACKGROUND = -1,
  NORMAL = 0,
  FROZEN = 1,
  SPEECH = 98,
  UI = 99
};

struct Entity : public sf::Drawable {
  DrawLayer draw_layer;  // default is NORMAL (0)
  bool cleanup;          // if this entity should be scheduled for cleanup

  std::vector<sf::Shape*> colliders;

  virtual float getDrawY() const = 0;

  bool operator<(const Entity& e) const {
    if (this->draw_layer == e.draw_layer) {
      return this->getDrawY() < e.getDrawY();
    }
    return this->draw_layer < e.draw_layer;
  }

  virtual void draw(sf::RenderTarget& target,
                    sf::RenderStates states) const = 0;

  bool checkCollision(const Entity& e) {
    if (this->draw_layer == DrawLayer::NORMAL &&
        e.draw_layer == DrawLayer::NORMAL) {
      // TODO decide if we want normal collision or not
      return false;
    }

    if (this->draw_layer != e.draw_layer) {
      // for this one special case, continue as normal
      if (this->draw_layer == DrawLayer::NORMAL &&
          e.draw_layer == DrawLayer::FROZEN) {
        // do nothing
      } else {
        return false;
      }
    }
    for (int i = 0; i < this->colliders.size(); i++) {
      for (int j = 0; j < e.colliders.size(); j++) {
        bool a = this->colliders[i]->getGlobalBounds().intersects(
            e.colliders[j]->getGlobalBounds());
        if (a) {
          return true;
        }
      }
    }
    return false;
  }

  virtual void update(sf::Time elapsed) {}

  virtual void handleCollision(Entity& e){
      // do nothing on collision by default
  };
};

#endif
