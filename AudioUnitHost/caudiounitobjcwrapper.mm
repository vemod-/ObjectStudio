#import "caudiounitobjcwrapper.h"

@implementation CAudioUnitObjCWrapper {
    NSMutableDictionary<NSNumber*, AUParameter*> *parameterMap;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        parameterMap = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)createUI:(UIView*)parentView audioUnit:(AVAudioUnit*)audioUnit{
    if (!parentView) return;

    NSArray *parameters = audioUnit.AUAudioUnit.parameterTree.allParameters;

    CGRect frame = parentView.frame;
    frame.size.height = (parameters.count * 30) + 40; // Dynamiskt höjd baserat på antalet sliders
    parentView.frame = frame;
    parentView.backgroundColor = [UIColor blackColor];

    for (uint i = 0; i < parameters.count; i++) {
        AUParameter *param = parameters[i];

       // Skapa en UILabel för parameter-namnet
        UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(20, (30 * i) + 20, 270, 20)];
        label.text = param.displayName;
        label.font = [UIFont systemFontOfSize:14];
        label.textColor = [UIColor whiteColor];

        // Skapa UISlider
        UISlider *slider = [[UISlider alloc] initWithFrame:CGRectMake(310, (30 * i) + 20, 270, 20)];
        slider.value = param.value;
        slider.tag = i;

        // Spara AUParameter
        parameterMap[@(i)] = param;

        // Lägg till event
        [slider addTarget:self action:@selector(sliderValueDidChange:) forControlEvents:UIControlEventValueChanged];

        [parentView addSubview:label];
        [parentView addSubview:slider];
    }
}

- (void)sliderValueDidChange:(UISlider*)slider {
    AUParameter *param = parameterMap[@(slider.tag)];
    if (param) {
        param.value = slider.value;
    }
}

@end
