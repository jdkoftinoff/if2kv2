
#include "jdk_world.h"
#include "jdk_settings.h"

int main( int argc, char **argv )
{
  jdk_settings_text settings(stdin);
  settings.save_file(stdout);
  return 0;
}
