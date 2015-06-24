//
//  SignalShared.h
//  Signal360
//
//  Created by Ryan Mannion on 11/8/11.
//  Copyright (c) 2011 Signal360 All rights reserved.
//

#import "SignalConstants.h"
#import "SignalCache.h"

#ifdef MANAGER
#import "SignalHistoricalData.h"
#endif

//Core Macros
#define SIGNAL_DEPRECATED __attribute__((deprecated))

//Notifications are used on delegate callbacks to enabled linked functionality
FOUNDATION_EXPORT NSString *const SignalNotificationDidHearCode;
FOUNDATION_EXPORT NSString *const SignalNotificationDidReceiveActivations;
FOUNDATION_EXPORT NSString *const SignalNotificationPreferencesUpdated;
FOUNDATION_EXPORT NSString *const SignalNotificationReset;
FOUNDATION_EXPORT NSString *const SignalNotificationRegionUpdated;
FOUNDATION_EXPORT NSString *const SignalNotificationLocationMessage;
FOUNDATION_EXPORT NSString *const SignalNotificationCustomPayload;
FOUNDATION_EXPORT NSString *const SignalUserInfoChannelCode;
FOUNDATION_EXPORT NSString *const SignalUserInfoCustomPayload;
FOUNDATION_EXPORT NSString *const SignalUserInfoContentId;

//Define Id Field Type
typedef long long int SignalId;

/**
 * Track user actions on content card
 */
typedef NS_ENUM(NSInteger, SignalUserActionType) {
    SwipeRight = 0,
    SwipeLeft = 1,
    ClickRightButton = 2,
    ClickLeftButton = 3,
    ClickOpenButton = 4,
    ClickCloseButton = 5,
    ClickAwayToClose = 6,
    NoticeAccept = 7,
    NoticeDecline = 8
};

#define signalUserActionTypeString(enum) [@[@"SWIPE_RIGHT", @"SWIPE_LEFT", @"CLICK_RIGHT_BUTTON", @"CLICK_LEFT_BUTTON", @"CLICK_OPEN_BUTTON", @"CLICK_CLOSE_BUTTON", @"CLICK_AWAY_TO_CLOSE", @"NOTICE_ACCEPT", @"NOTICE_DECLINE" ] objectAtIndex:enum];

//Declare these classes as they are used in delegates
@class Signal;
@class SignalActivation;
@class SignalLocation;
@class SignalCodeHeard;
@class SignalBluetoothCodeHeard;
@class SignalAudioCodeHeard;
@class SignalEvent;

/**
 * The SDK must be in one of the following statuses.
 * Note that each status implies that certain functionality maybe disabled.
 */
typedef NS_ENUM(NSInteger, SignalSdkStatus) {
    SignalSdkStatusNotInitialized, /** We have not been able to determine the status of the SDK, either the program is starting or there hasn't been any network connectivity.  Delegate will not receive any code or activation calls.*/
    SignalSdkStatusDisabled,        /** This SDK is disabled, the delegate will not receive any code or activation calls.*/
    SignalSdkStatusTrial,           /** This SDK is in a trial (developer) mode, a message will appear to the user.  Delegate will receive code and activation calls.*/
    SignalSdkStatusEnabled          /** This SDK is fully enabled, the delegate will receive all applicable calls.*/
};

/**
 * Result of manually refetching the content delivery configuration
 */
typedef NS_ENUM(NSInteger, SignalFetchResult) {
    SignalFetchResultNewData, /** A new configuration was received.*/
    SignalFetchResultNoData, /** No configuration changes were received.*/
    SignalFetchResultFailed /** Configuration refetch failed.*/
};

/**
 * Signal delegate used for responding to signal events, it includes several core functions
 *
 * - Responding to the fact that a signal is in range
 * - Responding to a specific signal with the appropriate time index relative to the program
 * - Respond to receiving activations (content)
 * - Respond to content being deleted
 * - Respond to offline content caching
 *
 */
@protocol SignalDelegate <NSObject>
@optional

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
- (BOOL) signal: (Signal *)signal didHearCode: (SignalCodeHeard *) code;


/**
 * Did receive activations is called after URL#signal:didHearCode:withTimeInterval returns YES.
 *
 * The server is then queried (or offline content prepared) and activations delivered
 *
 * @param signal - the configured running instance of signal
 * @param activations instances of SignalActivation that contain, delivery time, content, etc
 */
- (void) signal: (Signal *)signal didReceiveActivations: (NSArray *) activations;

/**
 * When offline content is receive and cached internally (signal only caches the raw data)
 * it is then passed to the delegate to have the implementing system cache whatever data is required.
 *
 * @param signal - the configured running instance of signal
 * @param activations is an array of SignalActivation that needs to have application level caching completed
 */
- (void) signal: (Signal *)signal cacheOfflineContent: (NSArray *) contents;

/**
 * When the SDK status changes, this method is executed with the new status
 *
 * @param signal - the configured running instance of signal
 * @param status new status of the SDK
 */
- (void) signal: (Signal *)signal didStatusChange: (SignalSdkStatus) status;

/**
 * When the application has location enabled, and the user has broken a geo-fence this will be called with that location.
 * It is not guaranteed that this is called when a user has entered a geo fence.
 *
 * @param signal - the configured running instance of signal
 * @param location of the geo fence entered
 */
- (void) signal: (Signal *)signal didGeoFenceEntered: (SignalLocation *) location;

/**
 * When the application has location enabled and the user has exited a geo-fence this will be called with that location.
 * It is not guaranteed that this is called for each geo fence enter event.  For example the user turned off device while inside
 * a geo fence.
 *
 * @param signal - the configured running instance of signal
 * @param location of the geo fence exited
 */
- (void) signal: (Signal *)signal didGeoFenceExited: (SignalLocation *) location;

/**
 * When the application has location enabled and sdk has determined that an updated set of locations should be monitored.
 *
 * @param signal - the configured running instance of signal
 * @param locations of the geo fences to monitor
 */
- (void) signal: (Signal *)signal didGeoFencesUpdated: (NSArray *) locations;

/**
 * When the application finishes registering
 *
 * @param signal - the configured running instance of signal
 * @param success whether or not sdk succeeded to register
 */
- (void) signal: (Signal *)signal didCompleteRegistration:(BOOL)success;

/**
 * When the application finishes configuring
 *
 * @param signal - the configured running instance of signal
 * @param changed whether or not configuration changed
 */
- (void) signal: (Signal *)signal didUpdateConfiguration:(BOOL)changed;

/**
 * Get tags for code
 *
 * @param signal - the configured running instance of signal
 * @param code code for which to provide tags
 */
- (NSDictionary*) signal: (Signal *)signal getTagsForCode:(SignalCodeHeard*)code;

@end

@protocol SignalActivationDelegate <NSObject>

/**
 * Whether or not we are allowed to activate the content
 *
 * @param signal - the configured running instance of signal
 * @param content the content we are trying to activate
 */
- (BOOL) signal: (Signal *)signal canActivateContent:(SignalActivation*)content;

@end

/**-----------------------------------------------------------------------------
 * @name Delegate
 * -----------------------------------------------------------------------------
 */



/** The `Signal` class manages the interaction with the Core Signal360 system.
 *
 *  Notifications used in delegate calls are below
 *  -SignalNotificationDidHearCode;
 *  -SignalNotificationDidReceiveActivations;
 *  -SignalNotificationPreferencesUpdated;
 *  -SignalNotificationReset;
 *  -SignalNotificationRegionUpdated;
 *  -SignalNotificationLocationMessage;
 *  -SignalNotificationCustomPayload;
 *  -SignalUserInfoChannelCode;
 *  -SignalUserInfoCustomPayload;
 *  -SignalUserInfoContentId;
 */
@interface Signal : NSObject <SignalCacheDownloaderDelegate>


/**-----------------------------------------------------------------------------
 * @name Core System Calls and Setup
 * -----------------------------------------------------------------------------
 */

/**
 * The delegate that was initialized
 */
@property (nonatomic, retain) id <SignalDelegate> delegate;

/**
 * The activation delegate
 */
@property (nonatomic, retain) id <SignalActivationDelegate> activationDelegate;

/**
 * Get the sdk singleton to execute operations on
 *
 * @return Signal singleton reference
 */
+ (Signal *) sharedInstance;


/**
 * Initialize the sdk with the application guid and a delegate that will receive all callbacks.
 *
 * @param applicationGuid unique identifier provided by Signal360 CMS
 * @param delegate for Signal360 callbacks
 * @param quiet request for sdk to prevent os popups
 */
- (void) initializeWithApplicationGUID: (NSString *) applicationGuid andDelegate: (id <SignalDelegate>) delegate quietOption:(BOOL) makeQuiet;

/**
 * Initialize the sdk with the application guid and a delegate that will receive all callbacks.
 *
 * @param applicationGuid unique identifier provided by Signal360 CMS
 * @param delegate for Signal callbacks
 */
- (void) initializeWithApplicationGUID: (NSString *) applicationGuid andDelegate: (id <SignalDelegate>) delegate;


/**
 * Start, this is for both Bluetooth and Audio. If you are interested in one or the other this can be configured via the CMS.
 *
 * @return whether or not listening began
 */
- (void) start;


/**
 * Stop, this is for both Bluetooth and Audio. If you are interested in one or the other this can be configured via the CMS.
 */
- (void) stop;

- (void) changeApplicationGUID: (NSString *) guid;

/**
 * Called to determine if Bluetooth is enabled on the device
 * @return BOOL whether or not Bluetooth is Enabled
 */
- (BOOL) isBluetoothEnabled;

/**
 * @return BOOL whether or not force audio is off
 */
- (BOOL) isForceAudioOff;

/**
 * Force Audio Off, so that Audio is not used even if the Server States to use Audio
 * @param forceAudio whether or not to fully disable audio
 */
- (void) forceAudioOff:(BOOL) forceAudio;

/**
 * @return BOOL whether or not force location is off
 */
- (BOOL) isForceLocationOff;

/**
 * Force location service off, this is a good opt-out opportunity
 * for users
 *
 * @param forceLocationOff whether or not to fully disable location services
 */
- (void) forceLocationOff: (BOOL) forceLocationOff;

/**
 * @return BOOL whether or not background is forced off
 */
- (BOOL) isForceBackgroundOff;

/**
 * Force background service off, this is a good opt-out opportunity
 * for users. This will shut off all decoding by sdk, both audio and bluetooth.
 *
 * @param forceBackgroundOff whether or not to fully disable background
 */
- (void) forceBackgroundOff: (BOOL) forceBackgroundOff;

/**
 * @return BOOL whether or not background audio is forced off
 */
- (BOOL) isForceBackgroundAudioOff;

/**
 * This will overide the CMS value and prevent audio usage going from foreground to background.
 *
 * @param forceBackgroundAudioOff whether or not to fully disable background
 */
- (void) forceBackgroundAudioOff: (BOOL) forceBackgroundAudioOff;

/**
 * @return BOOL whether or not location audio is forced off
 */
- (BOOL) isForceLocationAudioOff;

/**
 * This will overide the CMS value and prevent audio usage in the background due to location updates
 *
 * @param forceLocationAudioOff whether or not to fully disable background
 */
- (void) forceLocationAudioOff: (BOOL) forceLocationAudioOff;

/**
 * @return BOOL whether or not bluetooth is forced off
 */
- (BOOL) isForceBluetoothOff;

/**
 * This will overide the CMS value and prevent bluetooth usage in the background
 *
 * @param forceBluetoothOff whether or not to fully disable bluetooth
 */
- (void) forceBluetoothOff: (BOOL) forceBluetoothOff;

/**
 * Disable advertising identifier; default value = false
 */
- (void) setAdvertisingIdentifierDisabled: (BOOL) advertisingIdentifierDisabled;

/**
 * @return BOOL whether or not advertising identifier is enabled
 */
- (BOOL) isAdvertisingIdentifierEnabled;

/**
 * This allows SDK integrator to pass in customer identifier
 * @param customerIdentifier
 */
- (void) setCustomerIdentifier: (NSString *) customerIdentifier;

/**
 * Reset all content, activations, cached content, etc
 */
- (void) reset;

/**
 * Manually update cached configuration
 */
- (void) checkConfig:(void (^)(SignalFetchResult))completionHandler;

/**
 * Retrieve the current status of the SDK.
 */
- (SignalSdkStatus) status;

/**-----------------------------------------------------------------------------
 * @name Content Calls
 * -----------------------------------------------------------------------------
 */

/**
 * Mark an activation as engaged
 *
 * @param content that was engaged
 */
- (void) engagedContent: (SignalActivation *) content;

/**
 * Simulate code heard
 *
 * @param code
 */
- (void) getActivationsWithCodeHeard:(SignalCodeHeard *) code;

/**
 * Retrieve an activation identified by its id
 * @param activationId of activation to retrieve
 */
- (SignalActivation *) activationForId: (SignalId) activationId;

/**
 * Mark content with user action
 *
 * @param content that was acted on by user (right_swipe, left_swipe, click, etc)
 */
- (void) userActionOnContent: (SignalActivation *) activation withAction: (SignalUserActionType) action;

- (void) deactivateActivation:(SignalActivation *)activation;

/**
 * All active activations
 */

- (NSArray*) allActiveContent;

/**
 * Access the unique identifier for this device (NOT UDID).
 */
@property (nonatomic, retain, readonly) NSString * uuid;

/**
 * Access the advertising identifier.
 */
@property (nonatomic, strong) NSString *advertisingIdentifier;

#ifdef MANAGER
-(void) setHistoricalData:(SignalHistoricalData*)historicalData;
#endif

@end
