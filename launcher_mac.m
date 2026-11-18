#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include "modrinth.h"

/* --- VISTA ANIMAZIONE INIZIALE (CUBO ISOMETRICO + PALLINI) --- */
@interface IntroView : NSView
@property (assign) int introFrame;
@property (assign) BOOL isDark;
@end

@implementation IntroView

- (void)drawGradientQuad:(CGContextRef)ctx
                      p0:(CGPoint)p0 c0:(NSColor *)c0
                      p1:(CGPoint)p1 c1:(NSColor *)c1
                      p2:(CGPoint)p2 c2:(NSColor *)c2
                      p3:(CGPoint)p3 c3:(NSColor *)c3 {
    CGContextSaveGState(ctx);
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx, p0.x, p0.y);
    CGContextAddLineToPoint(ctx, p1.x, p1.y);
    CGContextAddLineToPoint(ctx, p2.x, p2.y);
    CGContextAddLineToPoint(ctx, p3.x, p3.y);
    CGContextClosePath(ctx);
    CGContextClip(ctx);

    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    NSArray *colors = @[(__bridge id)c0.CGColor, (__bridge id)c2.CGColor];
    CGFloat *locs = (CGFloat *)calloc(2, sizeof(CGFloat));
    locs[0] = 0.0;
    locs[1] = 1.0;
    CGGradientRef grad = CGGradientCreateWithColors(space, (__bridge CFArrayRef)colors, locs);
    CGContextDrawLinearGradient(ctx, grad, p0, p2, 0);

    free(locs);
    CGGradientRelease(grad);
    CGColorSpaceRelease(space);
    CGContextRestoreGState(ctx);
}

- (void)drawRect:(NSRect)dirtyRect {
    CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
    NSRect rc = [self bounds];

    NSColor *bg = self.isDark
        ? [NSColor colorWithCalibratedRed:24/255.0 green:24/255.0 blue:28/255.0 alpha:1.0]
        : [NSColor colorWithCalibratedRed:242/255.0 green:242/255.0 blue:247/255.0 alpha:1.0];
    [bg setFill];
    NSRectFill(rc);

    CGFloat cx = rc.size.width / 2.0;
    CGFloat cy = rc.size.height / 2.0 + 15.0;

    float t = (float)self.introFrame / 95.0f;
    float globalAlpha = (t < 0.82f) ? 1.0f : (1.0f - (t - 0.82f) / 0.18f);
    if (globalAlpha < 0.0f) globalAlpha = 0.0f;

    if (t < 0.52f) {
        float pT = t / 0.52f;
        float spread = sinf(pT * 3.14159265f);
        float easeSpread = 1.0f - (1.0f - spread) * (1.0f - spread);

        for (int i = 0; i < 12; i++) {
            float angle = (6.2831853f / 12.0f) * i + pT * 2.2f;
            float dist = easeSpread * (55.0f + (i % 3) * 18.0f);
            CGFloat dx = cx + cosf(angle) * dist;
            CGFloat dy = cy + sinf(angle) * dist;
            CGFloat r = 5.0f + 4.0f * (1.0f - easeSpread);

            NSColor *dotCol = (i % 2 == 0)
                ? [NSColor colorWithCalibratedRed:0.0 green:140/255.0 blue:1.0 alpha:globalAlpha]
                : [NSColor colorWithCalibratedRed:52/255.0 green:199/255.0 blue:110/255.0 alpha:globalAlpha];
            [dotCol setFill];
            CGContextFillEllipseInRect(ctx, CGRectMake(dx - r, dy - r, r * 2.0, r * 2.0));
        }
    }

    if (t > 0.28f) {
        float cT = (t - 0.28f) / 0.30f;
        if (cT > 1.0f) cT = 1.0f;
        float easeScale = (cT < 0.5f) ? (2.0f * cT * cT) : (1.0f - powf(-2.0f * cT + 2.0f, 2.0f) / 2.0f);
        if (easeScale > 1.0f) easeScale = 1.0f;

        CGFloat S = 48.0 * easeScale;
        CGFloat dx = S * 0.8660254;
        CGFloat dy = S * 0.5;

        CGPoint pCenter = CGPointMake(cx, cy);
        CGPoint pTop    = CGPointMake(cx, cy + S);
        CGPoint pTopR   = CGPointMake(cx + dx, cy + dy);
        CGPoint pBotR   = CGPointMake(cx + dx, cy - dy);
        CGPoint pBot    = CGPointMake(cx, cy - S);
        CGPoint pBotL   = CGPointMake(cx - dx, cy - dy);
        CGPoint pTopL   = CGPointMake(cx - dx, cy + dy);

        [self drawGradientQuad:ctx
                            p0:pTop    c0:[NSColor colorWithCalibratedRed:80/255.0 green:230/255.0 blue:120/255.0 alpha:globalAlpha]
                            p1:pTopR   c1:[NSColor colorWithCalibratedRed:40/255.0 green:205/255.0 blue:165/255.0 alpha:globalAlpha]
                            p2:pCenter c2:[NSColor colorWithCalibratedRed:30/255.0 green:185/255.0 blue:185/255.0 alpha:globalAlpha]
                            p3:pTopL   c3:[NSColor colorWithCalibratedRed:52/255.0 green:215/255.0 blue:130/255.0 alpha:globalAlpha]];

        [self drawGradientQuad:ctx
                            p0:pTopL   c0:[NSColor colorWithCalibratedRed:45/255.0 green:200/255.0 blue:150/255.0 alpha:globalAlpha]
                            p1:pCenter c1:[NSColor colorWithCalibratedRed:25/255.0 green:175/255.0 blue:210/255.0 alpha:globalAlpha]
                            p2:pBot    c2:[NSColor colorWithCalibratedRed:0.0      green:110/255.0 blue:1.0       alpha:globalAlpha]
                            p3:pBotL   c3:[NSColor colorWithCalibratedRed:10/255.0 green:135/255.0 blue:240/255.0 alpha:globalAlpha]];

        [self drawGradientQuad:ctx
                            p0:pCenter c0:[NSColor colorWithCalibratedRed:25/255.0 green:175/255.0 blue:210/255.0 alpha:globalAlpha]
                            p1:pTopR   c1:[NSColor colorWithCalibratedRed:35/255.0 green:190/255.0 blue:175/255.0 alpha:globalAlpha]
                            p2:pBotR   c2:[NSColor colorWithCalibratedRed:15/255.0 green:95/255.0  blue:205/255.0 alpha:globalAlpha]
                            p3:pBot    c3:[NSColor colorWithCalibratedRed:0.0      green:110/255.0 blue:1.0       alpha:globalAlpha]];
    }

    NSColor *txtCol = self.isDark
        ? [NSColor colorWithCalibratedWhite:1.0 alpha:globalAlpha]
        : [NSColor colorWithCalibratedRed:28/255.0 green:28/255.0 blue:30/255.0 alpha:globalAlpha];
    NSDictionary *attrs = @{
        NSFontAttributeName: [NSFont boldSystemFontOfSize:17],
        NSForegroundColorAttributeName: txtCol
    };
    NSSize txtSize = [@"PortableMC" sizeWithAttributes:attrs];
    [@"PortableMC" drawAtPoint:CGPointMake(cx - txtSize.width / 2.0, cy - 82.0) withAttributes:attrs];
}
@end
/* --- DELEGATE PRINCIPALE --- */
@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@property (strong) IntroView *introView;
@property (strong) NSTimer *introTimer;
@property (strong) NSView *statusDot;
@property (strong) NSButton *btnTheme;
@property (strong) NSPopUpButton *popupAuth;
@property (strong) NSTextField *editUser;
@property (strong) NSButton *btnLogin;
@property (strong) NSPopUpButton *popupLoader;
@property (strong) NSPopUpButton *popupVersion;
@property (strong) NSButton *chkSnap;
@property (strong) NSButton *chkBeta;
@property (strong) NSButton *chkInstance;
@property (strong) NSButton *chkAutoClose;
@property (strong) NSTextField *editDir;
@property (strong) NSTextView *textLog;
@property (strong) NSButton *btnLaunch;
@property (strong) NSProgressIndicator *progressBar;

@property (strong) NSTask *activeTask;
@property (assign) BOOL isDark;
@property (assign) BOOL isRunning;
@property (assign) BOOL wasCancelled;
@property (assign) int appStatus;
@end

static const char *ALL_VERSIONS[] = {
    "26.3", "26.2", "26.1",
    "1.21.5", "1.21.4", "1.21.3", "1.21.2", "1.21.1", "1.21",
    "1.20.6", "1.20.5", "1.20.4", "1.20.3", "1.20.2", "1.20.1", "1.20",
    "1.19.4", "1.19.3", "1.19.2", "1.19.1", "1.19",
    "1.18.2", "1.18.1", "1.18",
    "1.17.1", "1.17",
    "1.16.5", "1.16.4", "1.16.3", "1.16.2", "1.16.1", "1.16",
    "1.15.2", "1.15.1", "1.15",
    "1.14.4", "1.14.3", "1.14.2", "1.14.1", "1.14",
    "1.13.2", "1.13.1", "1.13",
    "1.12.2", "1.12.1", "1.12",
    "1.11.2", "1.11.1", "1.11",
    "1.10.2", "1.10.1", "1.10",
    "1.9.4", "1.9.3", "1.9.2", "1.9.1", "1.9",
    "1.8.9", "1.8.8", "1.8.7", "1.8.6", "1.8.5", "1.8.4", "1.8.3", "1.8.2", "1.8.1", "1.8",
    "1.7.10", "1.7.9", "1.7.8", "1.7.7", "1.7.6", "1.7.5", "1.7.4", "1.7.2",
    "1.6.4", "1.6.2", "1.6.1",
    "1.5.2", "1.5.1",
    "1.4.7", "1.4.6", "1.4.5", "1.4.4", "1.4.2",
    "1.3.2", "1.3.1",
    "1.2.5", "1.2.4", "1.2.3", "1.2.2", "1.2.1",
    "1.1", "1.0",
    "26.4-rc1", "26.4-pre2", "26.4-pre1", "26w18a", "26w16a", "26w14a", "26w10a", "26w06a", "26w02a",
    "25w46a", "25w44a", "25w41a", "25w35a", "25w20a", "25w14craftmine", "25w08a", "25w04a",
    "1.21.4-rc3", "1.21.4-pre1", "24w46a", "24w45a", "24w44a", "24w40a", "24w33a",
    "24w14potato", "24w12a", "24w09a", "24w03a",
    "23w51b", "23w45a", "23w31a", "23w13a_or_b", "23w07a",
    "22w46a", "22w24a", "22w13oneblockatatime", "22w11a",
    "21w44a", "21w37a", "21w14a", "21w03a",
    "20w51a", "20w45a", "20w14infinite", "20w06a",
    "19w46b", "19w34a", "19w13b", "19w02a",
    "18w50a", "18w43c", "18w30b", "18w16a", "18w01a",
    "17w50a", "17w43b", "17w18b", "17w06a",
    "16w50a", "16w39c", "16w20a", "16w02a",
    "15w51b", "15w41b", "15w31c", "15w14a",
    "14w34d", "14w25b", "14w11b", "14w02c",
    "13w48b", "13w36b", "13w24b", "13w16b", "13w01b",
    "12w50b", "12w34b", "12w21b", "12w07b", "11w49a",
    "b1.8.1", "b1.8", "b1.7.3", "b1.7.2", "b1.7",
    "b1.6.6", "b1.6.5", "b1.6.4", "b1.6",
    "b1.5_01", "b1.5", "b1.4_01", "b1.4",
    "b1.3_01", "b1.3b", "b1.2_02", "b1.2",
    "b1.1_02", "b1.0_01", "b1.0",
    "a1.2.6", "a1.2.5", "a1.2.3_04", "a1.2.2b", "a1.2.0_02",
    "a1.1.2_01", "a1.1.0", "a1.0.17_04", "a1.0.15", "a1.0.11", "a1.0.4",
    "inf-20100618",
    "c0.30_01c", "c0.0.13a", "c0.0.11a",
    "rd-161348", "rd-160052", "rd-132328", "rd-132211",
    NULL
};

static BOOL is_snapshot_ver(const char *v) {
    return (strchr(v, 'w') != NULL || strstr(v, "pre") != NULL || strstr(v, "rc") != NULL ||
            strstr(v, "craftmine") != NULL || strstr(v, "potato") != NULL || strstr(v, "infinite") != NULL ||
            strstr(v, "oneblock") != NULL);
}

static BOOL is_beta_ver(const char *v) {
    return (*v == 'b' || *v == 'a' || *v == 'c' || strncmp(v, "rd-", 3) == 0 || strncmp(v, "inf-", 4) == 0);
}

static void mkdir_p_mac(const char *path) {
    char *tmp = (char *)calloc(1024, 1);
    if (!tmp) return;
    snprintf(tmp, 1024, "%s", path);
    size_t len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
    free(tmp);
}
@implementation AppDelegate

- (NSString *)configPath {
    return [NSHomeDirectory() stringByAppendingPathComponent:@".portablemc_gui.plist"];
}

- (void)saveConfig {
    NSDictionary *cfg = @{
        @"dark": @(self.isDark),
        @"auth": @([self.popupAuth indexOfSelectedItem]),
        @"user": [self.editUser stringValue] ?: @"",
        @"loader": @([self.popupLoader indexOfSelectedItem]),
        @"ver": [self.popupVersion titleOfSelectedItem] ?: @"26.3",
        @"dir": [self.editDir stringValue] ?: @"",
        @"instance": @([self.chkInstance state] == NSControlStateValueOn),
        @"autoclose": @([self.chkAutoClose state] == NSControlStateValueOn)
    };
    [cfg writeToFile:[self configPath] atomically:YES];
}

- (void)loadConfig {
    NSDictionary *cfg = [NSDictionary dictionaryWithContentsOfFile:[self configPath]];
    if (!cfg) return;
    if ([cfg[@"dark"] boolValue]) [self toggleTheme:nil];
    NSInteger authIdx = [cfg[@"auth"] integerValue];
    if (authIdx >= 0 && authIdx < 2) {
        [self.popupAuth selectItemAtIndex:authIdx];
        [self authChanged:nil];
    }
    if (cfg[@"user"]) [self.editUser setStringValue:cfg[@"user"]];
    NSInteger ldrIdx = [cfg[@"loader"] integerValue];
    if (ldrIdx >= 0 && ldrIdx < 5) [self.popupLoader selectItemAtIndex:ldrIdx];
    if (cfg[@"ver"]) [self.popupVersion selectItemWithTitle:cfg[@"ver"]];
    if (cfg[@"dir"] && [cfg[@"dir"] length] > 0) [self.editDir setStringValue:cfg[@"dir"]];
    [self.chkInstance setState:[cfg[@"instance"] boolValue] ? NSControlStateValueOn : NSControlStateValueOff];
    [self.chkAutoClose setState:[cfg[@"autoclose"] boolValue] ? NSControlStateValueOn : NSControlStateValueOff];
}

- (NSString *)effectiveGameDir {
    NSString *base = [self.editDir stringValue];
    if ([self.chkInstance state] == NSControlStateValueOn) {
        NSString *ver = [self.popupVersion titleOfSelectedItem] ?: @"26.3";
        return [[base stringByAppendingPathComponent:@"instances"] stringByAppendingPathComponent:ver];
    }
    return base;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    self.isDark = NO;
    self.isRunning = NO;
    self.appStatus = 0;

    NSRect frame = NSMakeRect(0, 0, 540, 480);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [self.window setTitle:@"PortableMC v2.0"];
    [self.window center];

    NSView *content = [self.window contentView];

    /* Header */
    NSTextField *lblTitle = [NSTextField labelWithString:@"PortableMC"];
    [lblTitle setFont:[NSFont boldSystemFontOfSize:20]];
    [lblTitle setFrame:NSMakeRect(22, 436, 130, 28)];
    [content addSubview:lblTitle];

    self.statusDot = [[NSView alloc] initWithFrame:NSMakeRect(142, 445, 10, 10)];
    [self.statusDot setWantsLayer:YES];
    self.statusDot.layer.cornerRadius = 5.0;
    self.statusDot.layer.backgroundColor = [[NSColor systemGreenColor] CGColor];
    [content addSubview:self.statusDot];

    self.btnTheme = [[NSButton alloc] initWithFrame:NSMakeRect(400, 436, 118, 28)];
    [self.btnTheme setTitle:@"Light Theme"];
    [self.btnTheme setBezelStyle:NSBezelStyleRounded];
    [self.btnTheme setTarget:self];
    [self.btnTheme setAction:@selector(toggleTheme:)];
    [content addSubview:self.btnTheme];

    /* Row 1: Auth + User + Login */
    self.popupAuth = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(20, 394, 135, 26) pullsDown:NO];
    [self.popupAuth addItemsWithTitles:@[@"Username", @"Microsoft"]];
    [self.popupAuth setTarget:self];
    [self.popupAuth setAction:@selector(authChanged:)];
    [content addSubview:self.popupAuth];

    self.editUser = [[NSTextField alloc] initWithFrame:NSMakeRect(165, 395, 352, 24)];
    [self.editUser setPlaceholderString:@"Enter offline username..."];
    [self.editUser setBezelStyle:NSTextFieldRoundedBezel];
    [content addSubview:self.editUser];

    self.btnLogin = [[NSButton alloc] initWithFrame:NSMakeRect(442, 393, 76, 28)];
    [self.btnLogin setTitle:@"Login"];
    [self.btnLogin setBezelStyle:NSBezelStyleRounded];
    [self.btnLogin setHidden:YES];
    [self.btnLogin setTarget:self];
    [self.btnLogin setAction:@selector(loginClicked:)];
    [content addSubview:self.btnLogin];

    /* Row 2: Mod Loader */
    NSTextField *lblLoader = [NSTextField labelWithString:@"Mod Loader"];
    [lblLoader setFrame:NSMakeRect(22, 356, 130, 22)];
    [content addSubview:lblLoader];

    self.popupLoader = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(163, 353, 356, 26) pullsDown:NO];
    [self.popupLoader addItemsWithTitles:@[@"Vanilla", @"fabric", @"forge", @"neoforge", @"quilt"]];
    [content addSubview:self.popupLoader];

    /* Row 3: Version + Snapshot + Beta */
    NSTextField *lblVer = [NSTextField labelWithString:@"Version"];
    [lblVer setFrame:NSMakeRect(22, 316, 130, 22)];
    [content addSubview:lblVer];

    self.popupVersion = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(163, 313, 200, 26) pullsDown:NO];
    [content addSubview:self.popupVersion];

    self.chkSnap = [NSButton checkboxWithTitle:@"Snapshot" target:self action:@selector(populateVersions)];
    [self.chkSnap setFrame:NSMakeRect(370, 315, 85, 22)];
    [content addSubview:self.chkSnap];

    self.chkBeta = [NSButton checkboxWithTitle:@"Beta" target:self action:@selector(populateVersions)];
    [self.chkBeta setFrame:NSMakeRect(460, 315, 60, 22)];
    [content addSubview:self.chkBeta];

    [self populateVersions];

    /* Row 4: Game Folder + Browse + Skin + Cape */
    NSTextField *lblDir = [NSTextField labelWithString:@"Game folder"];
    [lblDir setFrame:NSMakeRect(22, 276, 130, 22)];
    [content addSubview:lblDir];

    NSString *defaultDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Application Support/minecraft"];
    self.editDir = [[NSTextField alloc] initWithFrame:NSMakeRect(165, 275, 178, 24)];
    [self.editDir setStringValue:defaultDir];
    [self.editDir setBezelStyle:NSTextFieldRoundedBezel];
    [content addSubview:self.editDir];

    NSButton *btnBrowse = [[NSButton alloc] initWithFrame:NSMakeRect(346, 273, 58, 28)];
    [btnBrowse setTitle:@"Browse"];
    [btnBrowse setBezelStyle:NSBezelStyleRounded];
    [btnBrowse setTarget:self];
    [btnBrowse setAction:@selector(browseDir:)];
    [content addSubview:btnBrowse];

    NSButton *btnSkin = [[NSButton alloc] initWithFrame:NSMakeRect(406, 273, 55, 28)];
    [btnSkin setTitle:@"+ Skin"];
    [btnSkin setBezelStyle:NSBezelStyleRounded];
    [btnSkin setTarget:self];
    [btnSkin setAction:@selector(importSkin:)];
    [content addSubview:btnSkin];

    NSButton *btnCape = [[NSButton alloc] initWithFrame:NSMakeRect(463, 273, 56, 28)];
    [btnCape setTitle:@"+ Cape"];
    [btnCape setBezelStyle:NSBezelStyleRounded];
    [btnCape setTarget:self];
    [btnCape setAction:@selector(importCape:)];
    [content addSubview:btnCape];

    /* Row 5: Tools (+ Mod, Search Mods, Boost FPS, Isolated, Auto-Close) */
    NSButton *btnMod = [[NSButton alloc] initWithFrame:NSMakeRect(20, 235, 64, 28)];
    [btnMod setTitle:@"+ Mod"];
    [btnMod setBezelStyle:NSBezelStyleRounded];
    [btnMod setTarget:self];
    [btnMod setAction:@selector(importMod:)];
    [content addSubview:btnMod];

    NSButton *btnSearch = [[NSButton alloc] initWithFrame:NSMakeRect(86, 235, 102, 28)];
    [btnSearch setTitle:@"Search Mods"];
    [btnSearch setBezelStyle:NSBezelStyleRounded];
    [btnSearch setTarget:self];
    [btnSearch setAction:@selector(searchModsClicked:)];
    [content addSubview:btnSearch];

    NSButton *btnBoost = [[NSButton alloc] initWithFrame:NSMakeRect(190, 235, 96, 28)];
    [btnBoost setTitle:@"Boost FPS"];
    [btnBoost setBezelStyle:NSBezelStyleRounded];
    [btnBoost setTarget:self];
    [btnBoost setAction:@selector(boostFpsClicked:)];
    [content addSubview:btnBoost];

    self.chkInstance = [NSButton checkboxWithTitle:@"Isolated" target:nil action:nil];
    [self.chkInstance setFrame:NSMakeRect(294, 238, 78, 22)];
    [content addSubview:self.chkInstance];

    self.chkAutoClose = [NSButton checkboxWithTitle:@"Auto-Close" target:nil action:nil];
    [self.chkAutoClose setFrame:NSMakeRect(374, 238, 146, 22)];
    [self.chkAutoClose setState:NSControlStateValueOn];
    [content addSubview:self.chkAutoClose];

    /* Log Area */
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(22, 82, 496, 142)];
    [scroll setHasVerticalScroller:YES];
    [scroll setBorderType:NSBezelBorder];
    self.textLog = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 496, 142)];
    [self.textLog setEditable:NO];
    [self.textLog setFont:[NSFont fontWithName:@"Menlo" size:11.5]];
    [self.textLog setString:@"Ready.\n"];
    [scroll setDocumentView:self.textLog];
    [content addSubview:scroll];

    /* Launch & Progress Bar */
    self.btnLaunch = [[NSButton alloc] initWithFrame:NSMakeRect(22, 22, 496, 44)];
    [self.btnLaunch setTitle:@"LAUNCH MINECRAFT"];
    [self.btnLaunch setBezelStyle:NSBezelStyleRounded];
    [self.btnLaunch setFont:[NSFont boldSystemFontOfSize:14]];
    [self.btnLaunch setTarget:self];
    [self.btnLaunch setAction:@selector(launchClicked:)];
    [content addSubview:self.btnLaunch];

    self.progressBar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(22, 32, 496, 24)];
    [self.progressBar setStyle:NSProgressIndicatorStyleBar];
    [self.progressBar setIndeterminate:YES];
    [self.progressBar setHidden:YES];
    [content addSubview:self.progressBar];

    [self loadConfig];

    /* Avvia Animazione Iniziale */
    self.introView = [[IntroView alloc] initWithFrame:[content bounds]];
    self.introView.introFrame = 0;
    self.introView.isDark = self.isDark;
    [content addSubview:self.introView];
    self.introTimer = [NSTimer scheduledTimerWithTimeInterval:0.033
                                                       target:self
                                                     selector:@selector(tickIntro:)
                                                     userInfo:nil
                                                      repeats:YES];

    [self.window makeKeyAndOrderFront:nil];
}

- (void)tickIntro:(NSTimer *)timer {
    self.introView.introFrame++;
    if (self.introView.introFrame >= 95) {
        [self.introTimer invalidate];
        self.introTimer = nil;
        [self.introView removeFromSuperview];
        self.introView = nil;
    } else {
        [self.introView setNeedsDisplay:YES];
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    [self saveConfig];
    return YES;
}

- (void)updateStatusDot {
    if (self.appStatus == 0) self.statusDot.layer.backgroundColor = [[NSColor systemGreenColor] CGColor];
    else if (self.appStatus == 1) self.statusDot.layer.backgroundColor = [[NSColor systemOrangeColor] CGColor];
    else self.statusDot.layer.backgroundColor = [[NSColor systemRedColor] CGColor];
}

- (void)appendLog:(NSString *)text {
    dispatch_async(dispatch_get_main_queue(), ^{
        if ([[self.textLog string] length] > 25000) {
            [self.textLog setString:@""];
        }
        NSAttributedString *attr = [[NSAttributedString alloc] initWithString:text
                                                                   attributes:@{ NSForegroundColorAttributeName: [NSColor labelColor],
                                                                                 NSFontAttributeName: [NSFont fontWithName:@"Menlo" size:11.5] }];
        [[self.textLog textStorage] appendAttributedString:attr];
        [self.textLog scrollRangeToVisible:NSMakeRange([[self.textLog string] length], 0)];
    });
}

- (void)showAlert:(NSString *)title message:(NSString *)msg folder:(NSString *)folderPath {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:title];
    [alert setInformativeText:msg];
    [alert addButtonWithTitle:@"OK"];
    if (folderPath) {
        [alert addButtonWithTitle:@"Open Folder"];
    }
    NSModalResponse res = [alert runModal];
    if (res == NSAlertSecondButtonReturn && folderPath) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:folderPath]];
    }
}

- (void)toggleTheme:(id)sender {
    self.isDark = !self.isDark;
    if (self.isDark) {
        self.window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
        [self.btnTheme setTitle:@"Dark Theme"];
    } else {
        self.window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
        [self.btnTheme setTitle:@"Light Theme"];
    }
}

- (void)authChanged:(id)sender {
    if ([self.popupAuth indexOfSelectedItem] == 1) {
        [self.btnLogin setHidden:NO];
        [self.editUser setFrame:NSMakeRect(165, 395, 270, 24)];
        [self.editUser setPlaceholderString:@"Optional Microsoft nickname..."];
    } else {
        [self.btnLogin setHidden:YES];
        [self.editUser setFrame:NSMakeRect(165, 395, 352, 24)];
        [self.editUser setPlaceholderString:@"Enter offline username..."];
    }
}

- (void)populateVersions {
    int showSnap = ([self.chkSnap state] == NSControlStateValueOn) ? 1 : 0;
    int showBeta = ([self.chkBeta state] == NSControlStateValueOn) ? 1 : 0;

    NSString *prevSelection = [self.popupVersion titleOfSelectedItem];
    [self.popupVersion removeAllItems];

    char (*dynVersions)[64] = calloc(1024, sizeof(*dynVersions));
    int fetched = 0;
    if (dynVersions) {
        fetched = FetchMojangVersions(dynVersions, 1024, showSnap, showBeta);
    }

    if (fetched > 0) {
        for (int i = 0; i < fetched; i++) {
            [self.popupVersion addItemWithTitle:[NSString stringWithUTF8String:dynVersions[i]]];
        }
    } else {
        /* Fallback offline se non c'è connessione */
        for (int i = 0; ALL_VERSIONS[i] != NULL; i++) {
            const char *v = ALL_VERSIONS[i];
            if (is_snapshot_ver(v) && !showSnap) continue;
            if (is_beta_ver(v) && !showBeta) continue;
            [self.popupVersion addItemWithTitle:[NSString stringWithUTF8String:v]];
        }
    }

    if (dynVersions) free(dynVersions);

    if (prevSelection && [self.popupVersion itemWithTitle:prevSelection]) {
        [self.popupVersion selectItemWithTitle:prevSelection];
    }
}

- (void)browseDir:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setAllowsMultipleSelection:NO];
    if ([panel runModal] == NSModalResponseOK) {
        NSURL *url = [[panel URLs] firstObject];
        if (url) [self.editDir setStringValue:[url path]];
    }
}

- (void)boostFpsClicked:(id)sender {
    if ([self.popupLoader indexOfSelectedItem] == 0) {
        [self showAlert:@"Select a Mod Loader"
                message:@"Boost FPS requires a Mod Loader (select 'fabric' recommended, or forge/neoforge/quilt)."
                 folder:nil];
        return;
    }
    NSString *baseDir = [self effectiveGameDir];
    mkdir_p_mac([baseDir UTF8String]);
    const char *ldr = [[self.popupLoader titleOfSelectedItem] UTF8String];
    const char *ver = [[self.popupVersion titleOfSelectedItem] UTF8String];

    [self appendLog:@"[Boost FPS] Installing optimization pack (Sodium, Lithium, FerriteCore, EntityCulling)...\n"];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        int count = InstallBoostFPSPack([baseDir UTF8String], ldr, ver);
        dispatch_async(dispatch_get_main_queue(), ^{
            NSString *modsDir = [baseDir stringByAppendingPathComponent:@"mods"];
            NSString *msg = [NSString stringWithFormat:@"Installed %d optimization mods for %s (%s)!", count, ver, ldr];
            [self appendLog:[NSString stringWithFormat:@"[Boost FPS] %@\n", msg]];
            [self showAlert:@"Boost FPS Ready" message:msg folder:modsDir];
        });
    });
}

- (void)searchModsClicked:(id)sender {
    if ([self.popupLoader indexOfSelectedItem] == 0) {
        [self showAlert:@"Select a Mod Loader"
                message:@"Please select Fabric, Forge, NeoForge, or Quilt before searching for mods."
                 folder:nil];
        return;
    }

    NSAlert *searchBox = [[NSAlert alloc] init];
    [searchBox setMessageText:@"Search Modrinth Catalog"];
    [searchBox setInformativeText:@"Enter mod name (e.g. sodium, iris, jei, xaero):"];
    [searchBox addButtonWithTitle:@"Search"];
    [searchBox addButtonWithTitle:@"Cancel"];

    NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 280, 24)];
    [input setPlaceholderString:@"Mod name..."];
    [searchBox setAccessoryView:input];

    if ([searchBox runModal] != NSAlertFirstButtonReturn) return;
    NSString *query = [input stringValue];
    if ([query length] == 0) return;

    const char *ldr = [[self.popupLoader titleOfSelectedItem] UTF8String];
    const char *ver = [[self.popupVersion titleOfSelectedItem] UTF8String];

    char (*slugs)[64] = calloc(8, sizeof(*slugs));
    char (*titles)[128] = calloc(8, sizeof(*titles));
    int found = SearchModrinthProjects([query UTF8String], ldr, ver, slugs, titles, 8);
    if (found <= 0) {
        free(slugs);
        free(titles);
        [self showAlert:@"No Results" message:@"No compatible mods found on Modrinth for this version/loader." folder:nil];
        return;
    }

    NSAlert *pickBox = [[NSAlert alloc] init];
    [pickBox setMessageText:@"Select Mod to Install"];
    [pickBox setInformativeText:[NSString stringWithFormat:@"Results for '%@' (%s - %s):", query, ldr, ver]];
    [pickBox addButtonWithTitle:@"Install Selected"];
    [pickBox addButtonWithTitle:@"Cancel"];

    NSPopUpButton *dropdown = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 280, 26) pullsDown:NO];
    for (int i = 0; i < found; i++) {
        [dropdown addItemWithTitle:[NSString stringWithUTF8String:titles[i]]];
    }
    [pickBox setAccessoryView:dropdown];

    if ([pickBox runModal] == NSAlertFirstButtonReturn) {
        NSInteger idx = [dropdown indexOfSelectedItem];
        if (idx >= 0 && idx < found) {
            NSString *baseDir = [self effectiveGameDir];
            mkdir_p_mac([baseDir UTF8String]);
            NSString *chosenTitle = [NSString stringWithUTF8String:titles[idx]];
            NSString *chosenSlug = [NSString stringWithUTF8String:slugs[idx]];

            [self appendLog:[NSString stringWithFormat:@"[Modrinth] Downloading %@...\n", chosenTitle]];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                int ok = DownloadModFromModrinth([chosenSlug UTF8String], [baseDir UTF8String], ldr, ver);
                dispatch_async(dispatch_get_main_queue(), ^{
                    NSString *modsDir = [baseDir stringByAppendingPathComponent:@"mods"];
                    if (ok) {
                        [self showAlert:@"Mod Installed"
                                message:[NSString stringWithFormat:@"'%@' was downloaded and installed!", chosenTitle]
                                 folder:modsDir];
                    } else {
                        [self showAlert:@"Download Failed" message:@"Could not download the selected mod." folder:nil];
                    }
                });
            });
        }
    }
    free(slugs);
    free(titles);
}
- (void)importMod:(id)sender {
    NSString *baseDir = [self effectiveGameDir];
    if ([baseDir length] == 0) {
        [self showAlert:@"Missing Game Folder" message:@"Please select a destination game folder first." folder:nil];
        return;
    }

    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowedFileTypes:@[@"jar"]];
    if ([panel runModal] == NSModalResponseOK) {
        NSURL *srcUrl = [[panel URLs] firstObject];
        NSString *modsDir = [baseDir stringByAppendingPathComponent:@"mods"];
        mkdir_p_mac([modsDir UTF8String]);
        NSString *dstPath = [modsDir stringByAppendingPathComponent:[srcUrl lastPathComponent]];
        NSError *err = nil;
        [[NSFileManager defaultManager] removeItemAtPath:dstPath error:nil];
        if ([[NSFileManager defaultManager] copyItemAtPath:[srcUrl path] toPath:dstPath error:&err]) {
            [self showAlert:@"Mod Imported" message:@"The .jar mod was copied to your mods folder." folder:modsDir];
        } else {
            [self showAlert:@"Import Error" message:@"Failed to copy the .jar mod file." folder:nil];
        }
    }
}

- (BOOL)hasCustomSkinLoaderInMods:(NSString *)modsDir {
    NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:modsDir error:nil];
    for (NSString *f in files) {
        if ([[f lowercaseString] hasPrefix:@"customskinloader"] && [[f lowercaseString] hasSuffix:@".jar"]) {
            return YES;
        }
    }
    return NO;
}

- (void)importSkinOrCape:(BOOL)isCape {
    if ([self.popupAuth indexOfSelectedItem] == 1) {
        NSString *mcUrl = [NSString stringWithFormat:@"https://www%@minecraft%@net", @".", @"."];
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:mcUrl]];
        return;
    }

    if ([self.popupLoader indexOfSelectedItem] == 0) {
        [self showAlert:@"Vanilla Not Supported"
                message:@"Offline skins and capes cannot be applied on Vanilla.\n\nPlease select Fabric, Forge, NeoForge, or Quilt first."
                 folder:nil];
        return;
    }

    NSString *baseDir = [self effectiveGameDir];
    if ([baseDir length] == 0) {
        [self showAlert:@"Missing Game Folder" message:@"Please select a destination game folder first." folder:nil];
        return;
    }

    NSString *user = [self.editUser stringValue];
    if ([user length] == 0) user = @"Steve";

    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowedFileTypes:@[@"png"]];
    if ([panel runModal] == NSModalResponseOK) {
        NSURL *srcUrl = [[panel URLs] firstObject];
        NSString *sub = isCape ? @"CustomSkinLoader/LocalSkin/capes" : @"CustomSkinLoader/LocalSkin/skins";
        NSString *targetDir = [baseDir stringByAppendingPathComponent:sub];
        mkdir_p_mac([targetDir UTF8String]);

        NSString *dstPng = [targetDir stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.png", user]];
        [[NSFileManager defaultManager] removeItemAtPath:dstPng error:nil];
        if ([[NSFileManager defaultManager] copyItemAtPath:[srcUrl path] toPath:dstPng error:nil]) {
            NSString *jsonPath = [baseDir stringByAppendingPathComponent:@"CustomSkinLoader/CustomSkinLoader.json"];
            if (![[NSFileManager defaultManager] fileExistsAtPath:jsonPath]) {
                NSString *cfg = @"{\n"
                                 "  \"enable\": true,\n"
                                 "  \"enableDynamicSkull\": true,\n"
                                 "  \"enableTransparentSkin\": true,\n"
                                 "  \"forceLoadAllTextures\": true,\n"
                                 "  \"enableLocalProfile\": true,\n"
                                 "  \"loadlist\": [\n"
                                 "    {\n"
                                 "      \"name\": \"LocalSkin\",\n"
                                 "      \"type\": \"Legacy\",\n"
                                 "      \"checkPNG\": false,\n"
                                 "      \"skin\": \"LocalSkin/skins/{USERNAME}.png\",\n"
                                 "      \"cape\": \"LocalSkin/capes/{USERNAME}.png\",\n"
                                 "      \"elytra\": \"LocalSkin/elytras/{USERNAME}.png\"\n"
                                 "    }\n"
                                 "  ]\n"
                                 "}\n";
                [cfg writeToFile:jsonPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
            }

            NSString *modsDir = [baseDir stringByAppendingPathComponent:@"mods"];
            mkdir_p_mac([modsDir UTF8String]);

            NSString *statusNote = @"CustomSkinLoader is already installed in your mods folder.";
            if (![self hasCustomSkinLoaderInMods:modsDir]) {
                const char *loaderStr = [[self.popupLoader titleOfSelectedItem] UTF8String];
                const char *verStr = [[self.popupVersion titleOfSelectedItem] UTF8String];
                int downloaded = DownloadModFromModrinth("customskinloader", [baseDir UTF8String], loaderStr, verStr);
                if (downloaded) {
                    statusNote = @"CustomSkinLoader was automatically downloaded to your mods folder!";
                } else {
                    statusNote = @"Could not auto-download CustomSkinLoader. Please import the .jar manually via '+ Mod'.";
                }
            }

            NSString *finalMsg = [NSString stringWithFormat:@"Texture saved for '%@'!\n\n%@", user, statusNote];
            [self showAlert:(isCape ? @"Cape Imported" : @"Skin Imported")
                    message:finalMsg
                     folder:targetDir];
        } else {
            [self showAlert:@"Import Error" message:@"Failed to copy the .png file." folder:nil];
        }
    }
}

- (void)importSkin:(id)sender { [self importSkinOrCape:NO]; }
- (void)importCape:(id)sender { [self importSkinOrCape:YES]; }

- (NSString *)resolvePmcBinary {
    NSString *bundleDir = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
    NSArray *candidates = @[
        [bundleDir stringByAppendingPathComponent:@"portablemc"],
        [bundleDir stringByAppendingPathComponent:@"portablemc-cli"],
        @"/opt/homebrew/bin/portablemc",
        @"/usr/local/bin/portablemc",
        @"/usr/bin/portablemc"
    ];
    for (NSString *path in candidates) {
        if ([[NSFileManager defaultManager] isExecutableFileAtPath:path]) {
            return path;
        }
    }
    return nil;
}
- (void)runCliArgs:(NSArray<NSString *> *)args isGameLaunch:(BOOL)isGameLaunch {
    NSString *bin = [self resolvePmcBinary];
    if (!bin) {
        [self appendLog:@"[ERROR] 'portablemc' binary not found. Install it via 'pip3 install portablemc' or place it next to PortableMC-GUI.app.\n"];
        self.appStatus = 2;
        [self updateStatusDot];
        return;
    }

    self.isRunning = YES;
    self.wasCancelled = NO;
    self.appStatus = 1;
    [self updateStatusDot];

    [self.btnLaunch setTitle:@"RUNNING... (Click to Cancel)"];
    [self.progressBar setHidden:NO];
    [self.progressBar startAnimation:nil];

    NSTask *task = [[NSTask alloc] init];
    [task setExecutableURL:[NSURL fileURLWithPath:bin]];
    [task setArguments:args];

    NSPipe *pipe = [NSPipe pipe];
    [task setStandardOutput:pipe];
    [task setStandardError:pipe];

    __weak AppDelegate *weakSelf = self;
    NSFileHandle *fh = [pipe fileHandleForReading];
    fh.readabilityHandler = ^(NSFileHandle *handle) {
        NSData *data = [handle availableData];
        if ([data length] > 0) {
            NSString *str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            if (str) {
                [weakSelf appendLog:str];
                if (isGameLaunch && ([str containsString:@"Launching "] || [str containsString:@"Setting user:"])) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if ([weakSelf.chkAutoClose state] == NSControlStateValueOn) {
                            [weakSelf saveConfig];
                            [NSApp terminate:nil];
                        }
                    });
                }
            }
        }
    };

    task.terminationHandler = ^(NSTask *t) {
        fh.readabilityHandler = nil;
        dispatch_async(dispatch_get_main_queue(), ^{
            weakSelf.isRunning = NO;
            weakSelf.activeTask = nil;
            [weakSelf.progressBar stopAnimation:nil];
            [weakSelf.progressBar setHidden:YES];
            [weakSelf.btnLaunch setTitle:@"LAUNCH MINECRAFT"];

            if (weakSelf.wasCancelled) {
                weakSelf.appStatus = 2;
                [weakSelf appendLog:@"\n--- Process cancelled by user ---\n"];
                [weakSelf showAlert:@"Download Cancelled"
                            message:@"The process was stopped.\nCheck your game folder if a download was interrupted."
                             folder:[weakSelf effectiveGameDir]];
            } else if ([t terminationStatus] == 0) {
                weakSelf.appStatus = 0;
                [weakSelf appendLog:@"\n--- Process finished ---\n"];
            } else {
                weakSelf.appStatus = 2;
                [weakSelf appendLog:@"\n--- Process exited with errors ---\n"];
            }
            [weakSelf updateStatusDot];
        });
    };

    self.activeTask = task;
    NSError *err = nil;
    if (![task launchAndReturnError:&err]) {
        self.isRunning = NO;
        self.appStatus = 2;
        [self updateStatusDot];
        [self.progressBar stopAnimation:nil];
        [self.progressBar setHidden:YES];
        [self.btnLaunch setTitle:@"LAUNCH MINECRAFT"];
        [self appendLog:[NSString stringWithFormat:@"[ERROR] %@\n", [err localizedDescription]]];
    }
}

- (void)loginClicked:(id)sender {
    if (self.isRunning) return;
    NSString *dir = [self effectiveGameDir];
    mkdir_p_mac([dir UTF8String]);
    [self appendLog:@"Starting Microsoft authentication...\n"];
    NSString *loginUrl = [NSString stringWithFormat:@"https://www%@microsoft%@com/link", @".", @"."];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:loginUrl]];
    [self runCliArgs:@[@"--main-dir", dir, @"auth", @"login", @"--no-browser"] isGameLaunch:NO];
}

- (void)launchClicked:(id)sender {
    if (self.isRunning && self.activeTask) {
        self.wasCancelled = YES;
        [self.activeTask terminate];
        return;
    }

    [self saveConfig];
    NSString *dir = [self effectiveGameDir];
    NSString *user = [self.editUser stringValue];
    NSString *loader = [self.popupLoader titleOfSelectedItem];
    NSString *ver = [self.popupVersion titleOfSelectedItem];
    mkdir_p_mac([dir UTF8String]);

    NSString *fullVer = ([loader isEqualToString:@"Vanilla"]) ? ver : [NSString stringWithFormat:@"%@:%@", loader, ver];
    NSMutableArray *args = [NSMutableArray arrayWithObjects:@"--main-dir", dir, @"start", nil];

    if ([self.popupAuth indexOfSelectedItem] == 1) {
        [args addObject:@"-a"];
        if ([user length] > 0) {
            [args addObject:@"-u"];
            [args addObject:user];
        }
    } else {
        [args addObject:@"-u"];
        [args addObject:([user length] > 0 ? user : @"Player")];
    }
    [args addObject:fullVer];

    [self appendLog:@"Starting process...\n"];
    [self runCliArgs:args isGameLaunch:YES];
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];
        [app activateIgnoringOtherApps:YES];
        [app run];
    }
    return 0;
}
