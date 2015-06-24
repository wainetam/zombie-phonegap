//
//  SignalPG.m
//  signal-phonegap
//
//  Created by Waine Tam on 6/15/15.
//
//

#import "SignalPG.h"

@implementation SignalPG

#pragma mark Signal methods for Cordova JS Bridge

- (void) initialize:(CDVInvokedUrlCommand *)command {
    NSString *applicationGuid = [command.arguments objectAtIndex:0];
    BOOL makeQuiet = false;
    if([command.arguments count] > 1) {
        makeQuiet = [command.arguments objectAtIndex:1 withDefault:nil];
    }
    
    [[SignalUI sharedInstance] initializeWithDelegate:[[UIApplication sharedApplication] delegate]];
    [[Signal sharedInstance] initializeWithApplicationGUID:applicationGuid andDelegate:[[UIApplication sharedApplication] delegate] quietOption:makeQuiet];
};

- (void) start:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] start];
}

- (void) stop:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] stop];
}

/**
 * Called to determine if start has been called
 * @return BOOL whether or not start has been called
 */
- (void) isOn:(CDVInvokedUrlCommand *)command {
    BOOL on = [[Signal sharedInstance] isOn];
    CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsBool:on];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
};

/**
 * Called to determine if Bluetooth is enabled on the device
 * @return BOOL whether or not Bluetooth is Enabled
 */
- (void) isBluetoothEnabled:(CDVInvokedUrlCommand *)command {
    BOOL enabled = [[Signal sharedInstance] isBluetoothEnabled];
    CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsBool:enabled];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void) userOptOut:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] userOptOut];
}

- (void) userOptIn:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] userOptIn];
}

/**
 * @return BOOL whether or not user has opted out
 */
- (void) isUserOptedOut:(CDVInvokedUrlCommand *)command {
    BOOL *optedOut = [[Signal sharedInstance] isUserOptedOut];
    CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsBool:optedOut];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void) enableAdvertisingIdentifier:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] enableAdvertisingIdentifier];
}

- (void) disableAdvertisingIdentifier:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] disableAdvertisingIdentifier];
}

/**
 * @return BOOL whether or not advertising identifier is enabled
 */
- (void) isAdvertisingIdentifierEnabled:(CDVInvokedUrlCommand *)command {
    BOOL *enabled = [[Signal sharedInstance] useAdvertisingIdentifier];
    CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsBool:enabled];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void) setCustomerIdentifier:(CDVInvokedUrlCommand *)command {
    NSString *customerIdentifier = [command.arguments objectAtIndex:0];
    customerIdentifier = ((customerIdentifier == [NSNull null] || customerIdentifier.length == 0) ? nil : customerIdentifier);
    if(customerIdentifier) {
        [[Signal sharedInstance] setCustomerIdentifier:customerIdentifier];
    }
}

- (void) reset:(CDVInvokedUrlCommand *)command {
    [[Signal sharedInstance] reset];
}

//- (void) checkConfig:(void (^)(SignalFetchResult))completionHandler;
- (void) checkConfig:(CDVInvokedUrlCommand *)command {
    // how to ensure 'block function' written in JS is converted to Obj C object?
    void (^completionHandler)(SignalFetchResult) = [command.arguments objectAtIndex:0 withDefault: nil];
    [[Signal sharedInstance] checkConfig:completionHandler];
}

- (void) getActivationsWithCodeHeard:(CDVInvokedUrlCommand *)command {
    int beaconCode = [command.arguments objectAtIndex:0];
    
    if (beaconCode != 0) {
        [[Signal sharedInstance] getActivationsWithCodeHeard:[[SignalCodeHeard alloc] initWithBeaconCode:beaconCode]];
    }
}

/**
 * All active activations
 */

- (void) allActiveContent:(CDVInvokedUrlCommand *)command {
    NSArray *content = [[Signal sharedInstance] allActiveContent];
    CDVPluginResult *pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsArray:content];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

#pragma mark SignalDelegate methods for Cordova JS Bridge

- (NSString *)serializeSignalCodeHeard:(SignalCodeHeard *)code {
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    
    if ([code respondsToSelector:@selector(beaconCode)]) {
        [dict setObject:code.beaconCode forKey:@"beaconCode"];
    }
    
    if ([code respondsToSelector:@selector(timeCodeHeard)]) {
        [dict setObject:code.timeCodeHeard forKey:@"timeCodeHeard"];
    }
    
    if ([code respondsToSelector:@selector(codesHeard)]) {
        [dict setObject:code.codesHeard forKey:@"codesHeard"];
    }
    
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:[dict copy] options:0 error:&error];
    NSString *jsonString = nil;
    
    if (!jsonData) {
        NSLog(@"Got an error: %@", error);
        jsonString = [[NSString alloc] initWithString:[error localizedDescription]];
    } else {
        jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    }
    
    return jsonString;
}

/**
 * This is called when a signal is heard and provides a code heard object
 *
 * NOTE: this does not mean content is available
 *
 * @param signal - the configured running instance of signal
 * @param codeHeard instance of the SignalCodeHeard
 *
 * @return whether or not you are interested in receiving content for this signal, it is the implementers responsibility for throttling
 *
 */
- (BOOL) signal: (Signal *)signal didHearCode: (SignalCodeHeard *) code {
    NSString *jsString = nil;

    NSString *jsonString = [self serializeSignalCodeHeard:code];
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidHearCodeCall(\"%@\");", jsonString]; // serialize SignalCodeHeard
    [self.commandDelegate evalJs:jsString];
}

/**
 * Did receive activations is called after URL#signal:didHearCode:withTimeInterval returns YES.
 *
 * The server is then queried (or offline content prepared) and activations delivered
 *
 * @param signal - the configured running instance of signal
 * @param activations instances of SignalActivation that contain, delivery time, content, etc
 */
- (void) signal: (Signal *)signal didReceiveActivations: (NSArray *) activations {
    NSString *jsString = nil;
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidReceiveActivationsCall(\"%@\");", [activations description]]; // array of SignalActivation
    [self.commandDelegate evalJs:jsString];
}

/**
 * When the SDK status changes, this method is executed with the new status
 *
 * @param signal - the configured running instance of signal
 * @param status new status of the SDK
 */
- (void) signal: (Signal *)signal didStatusChange: (SignalSdkStatus) status {
    NSString *jsString = nil;
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidStatusChange(\"%ld\");", status]; // SignalSdkStatus is an NSInteger
    [self.commandDelegate evalJs:jsString];
}

- (NSString *)serializeSignalLocation:(SignalLocation *) location {
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
    dateFormatter.dateFormat = @"yyyy-MM-dd HH:mm:ss ZZZ";
    
    if ([location respondsToSelector:@selector(identifier)]) {
        [dict setObject:location.identifier forKey:@"identifier"];
    }
    
    if ([location respondsToSelector:@selector(name)]) {
        [dict setObject:location.name forKey:@"name"];
    }
    
    if ([location respondsToSelector:@selector(endTime)]) {
        NSString* endTimeString = [dateFormatter stringFromDate:location.endTime];
        [dict setObject:endTimeString forKey:@"endTime"];
    }
    
    if ([location respondsToSelector:@selector(startTime)]) {
        NSString* startTimeString = [dateFormatter stringFromDate:location.startTime];
        [dict setObject:startTimeString forKey:@"startTime"];
    }
    
//    if ([location respondsToSelector:@selector(latitude)]) {
//        [dict setObject:location.latitude forKey:@"latitude"];
//    }
//    
//    if ([location respondsToSelector:@selector(longitude)]) {
//        [dict setObject:location.longitude forKey:@"longitude"];
//    }
//    
//    if ([location respondsToSelector:@selector(radius)]) {
//        [dict setObject:location.radius forKey:@"radius"];
//    }
    
    if ([location respondsToSelector:@selector(state)]) {
        [dict setObject:location.state forKey:@"state"];
    }
    
    if ([location respondsToSelector:@selector(programId)]) {
        [dict setObject:location.programId forKey:@"programId"];
    }
    
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:[dict copy] options:0 error:&error];
    NSString *jsonString = nil;
    
    if (!jsonData) {
        NSLog(@"Got an error: %@", error);
        jsonString = [[NSString alloc] initWithString:[error localizedDescription]];
    } else {
        jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    }
    
    return jsonString;
}

/**
 * When the application has location enabled, and the user has broken a geo-fence this will be called with that location.
 * It is not guaranteed that this is called when a user has entered a geo fence.
 *
 * @param signal - the configured running instance of signal
 * @param location of the geo fence entered
 */
- (void) signal: (Signal *)signal didGeoFenceEntered: (SignalLocation *) location {
    NSString *jsString = nil;
    NSString *jsonString = [self serializeSignalLocation:location];
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidGeoFenceEntered(\"%@\");", jsonString]; // serialize SignalLocation
    [self.commandDelegate evalJs:jsString];
}

/**
 * When the application has location enabled and the user has exited a geo-fence this will be called with that location.
 * It is not guaranteed that this is called for each geo fence enter event.  For example the user turned off device while inside
 * a geo fence.
 *
 * @param signal - the configured running instance of signal
 * @param location of the geo fence exited
 */
- (void) signal: (Signal *)signal didGeoFenceExited: (SignalLocation *) location {
    NSString *jsString = nil;
    NSString *jsonString = [self serializeSignalLocation:location];
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidGeoFenceExited(\"%@\");", jsonString]; // serialize SignalLocation
    [self.commandDelegate evalJs:jsString];
}

/**
 * When the application has location enabled and sdk has determined that an updated set of locations should be monitored.
 *
 * @param signal - the configured running instance of signal
 * @param locations of the geo fences to monitor
 */
- (void) signal: (Signal *)signal didGeoFencesUpdated: (NSArray *) locations {
    NSString *jsString = nil;
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidGeoFencesUpdated(\"%@\");", [locations description]]; // array of SignalLocation
    [self.commandDelegate evalJs:jsString];
}

/**
 * When the application finishes registering
 *
 * @param signal - the configured running instance of signal
 * @param success whether or not sdk succeeded to register
 */
- (void) signal: (Signal *)signal didCompleteRegistration:(BOOL)success {
    NSString *jsString = nil;
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidCompleteRegistration(\"%d\");", success];
    [self.commandDelegate evalJs:jsString];
}

/**
 * When the application finishes configuring
 *
 * @param signal - the configured running instance of signal
 * @param changed whether or not configuration changed
 */
- (void) signal: (Signal *)signal didUpdateConfiguration:(BOOL)changed {
    NSString *jsString = nil;
    jsString = [NSString stringWithFormat:@"SignalPG._nativeDidUpdateConfiguration(\"%d\");", changed];
    [self.commandDelegate evalJs:jsString];
}

/**
 * Get tags for code
 *
 * @param signal - the configured running instance of signal
 * @param code code for which to provide tags
 */
- (NSDictionary*) signal: (Signal *)signal getTagsForCode:(SignalCodeHeard*)code {
    NSString *jsString = nil;
    NSString *jsonString = [self serializeSignalCodeHeard:code];
    jsString = [NSString stringWithFormat:@"SignalPG._nativeGetTagsForCode(\"%d\");", jsonString]; // serialize SignalCodeHeard
    [self.commandDelegate evalJs:jsString];
}

@end
