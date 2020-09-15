#import "scoped_nsobject.h"
#import <Foundation/Foundation.h>

static bool s_didDealloc = false;

@interface Foobar : NSObject
@end

@implementation Foobar
-(void)dealloc {
  s_didDealloc = true;
}
@end

int main() {
  {
    fml::scoped_nsobject<Foobar> array([[Foobar alloc] init]);
  }
  printf("did dealloc: %d\n", s_didDealloc);
  return 0;
}