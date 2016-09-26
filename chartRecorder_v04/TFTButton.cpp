#include "Arduino.h"
#include "TFTButton.h"

TFTButton::TFTButton(int posx, int posy, int sizex, int sizey, tftt) {
  _posx = posx;
  _posy = posy;
  _sizex = sizex;
  _sizey = sizey;

}

boolean TFTButton::withinBounds(word x, word y, word bounds[]) {

}

void TFTButton::drawButton() {
  tft.graphicsMode();
  tft.fillRect(_posx, _posy, _sizex, _sizey, RA8875_WHITE)
}
