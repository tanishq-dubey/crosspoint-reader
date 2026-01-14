#include "BootActivity.h"

#include <GfxRenderer.h>

#include "fontIds.h"

void BootActivity::onEnter() {
  Activity::onEnter();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();

  // Draw "//DWS" large and centered
  renderer.drawCenteredText(BOOKERLY_18_FONT_ID, pageHeight / 2 - 20, "//DWS", true, EpdFontFamily::BOLD);
  // Draw "SignalOS" smaller below
  renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2 + 15, "SignalOS", true);

  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight / 2 + 50, "BOOTING");
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 30, SIGNALOS_VERSION);
  renderer.displayBuffer();
}
