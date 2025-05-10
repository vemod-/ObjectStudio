#ifndef CAUDIOUNITOBJCWRAPPER_H
#define CAUDIOUNITOBJCWRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <AVFAudio/AVFAudio.h>
#endif

@interface CAudioUnitObjCWrapper : NSObject

- (instancetype)init;
- (void)createUI:(UIView*)parentView audioUnit:(AVAudioUnit*)audioUnit;

@end

#ifdef __cplusplus
}
#endif

#endif // CAUDIOUNITOBJCWRAPPER_H
