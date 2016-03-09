//
//  ViewController.m
//  maxoubox
//
//  Created by Olivier on 09/03/2016.
//  Copyright © 2016 Maestun. All rights reserved.
//

#import "ViewController.h"
#import "maxoubox.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.

    int i = 0;
    for (UIButton * button in [[self uvButtons] subviews]) {
        [button setTitle:[NSString stringWithFormat:@"%d", i++] forState:UIControlStateNormal];
    }
    
    
    maxou_setup();
    
    [NSTimer scheduledTimerWithTimeInterval:0.01 target:self selector:@selector(loopThread) userInfo:nil repeats:YES];
}



- (void) loopThread {
    maxou_loop();
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
