#ifndef SPRITESHEET_HPP
#define SPRITESHEET_HPP

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <iostream>

const int SPRITE_SIZE = 100;
const int ITEMS_PER_ROW = 20;

struct _SpriteSheet {
    sf::Texture ss;
    sf::Image img;

    _SpriteSheet(){
        if (!ss.loadFromFile("./resources/tilesheet.png")){
            std::cout << "failed to load sprite sheet";
            std::cout << std::endl;
        } 
        img = ss.copyToImage();
    }

    sf::Color getColorAt(int x, int y){
        return img.getPixel(x, y);
    }

    sf::FloatRect getCollider(int index){
        int x = SPRITE_SIZE * (index % ITEMS_PER_ROW);
        int y = SPRITE_SIZE * (index / ITEMS_PER_ROW);

        int minX = SPRITE_SIZE;
        int minY = SPRITE_SIZE;
        int maxX = 0;
        int maxY = 0;

        for(int i=0; i < SPRITE_SIZE; i++){
            for(int j=0; j < SPRITE_SIZE; j++){
                sf::Color c = getColorAt(x+i, y+j);
                // transparent pixel
                if(c.a != 0){
                    if(i < minX){ minX = i; }
                    if(i > maxX){ maxX = i; }
                    if(j < minY){ minY = j; }
                    if(j > maxY){ maxY = j; }
                }
            }
        }
        return sf::FloatRect(minX, minY, maxX-minX, maxY-minY);
    }

    void texSprite(sf::Sprite* sprite, int index){
        int x = index % ITEMS_PER_ROW;
        int y = index / ITEMS_PER_ROW;
        texSpriteXY(sprite, x, y);
    }

    void texSpriteXY(sf::Sprite* sprite, int x, int y){
        sprite->setTexture(ss);
        sprite->setTextureRect(sf::IntRect(
            SPRITE_SIZE * x, SPRITE_SIZE * y, 
            SPRITE_SIZE, SPRITE_SIZE
            )
        );
    }
} SpriteSheet;

#endif
