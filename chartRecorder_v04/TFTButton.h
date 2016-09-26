/*
  TFTButton.h - Library for adding buttons to TFT touchscreens.
  Created by Brunston Poon, 2016.
             Space Sciences Laboratory
  Released under GNU GPL v3
*/

#ifndef TFTButton_h
#define TFTButton_h

#include "Arduino.h"

class TFTButton
{
  public:
    TFTButton(int posx, int posy, int sizex, int sizey, tftt);
    boolean withinBounds(word x, word y, word bounds[]);
    void drawButton();
  private:
    int _posx;
    int _posy;
    int _sizex;
    int _sizey;
    
    word _x;
    word _y;
    word _bounds[];
}

#endif
