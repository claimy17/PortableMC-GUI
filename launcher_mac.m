#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@property (strong) NSView *statusDot;
@property (strong) NSButton *btnTheme;
@property (strong) NSPopUpButton *popupAuth;
@property (strong) NSTextField *editUser;
@property (strong) NSButton *btnLogin;
@property (strong) NSPopUpButton *popupLoader;
@property (strong) NSPopUpButton *popupVersion;
@property (strong) NSButton *chkSnap;
@property (strong) NSButton *chkBeta;
@property (strong) NSTextField *editDir;
@property (strong) NSTextView *textLog;
@property (strong) NSButton *btnLaunch;
@property (strong) NSProgressIndicator *progressBar;

@property (strong) NSTask *activeTask;
@property (assign) BOOL isDark;
@property (assign) BOOL isRunning;
@property (assign) BOOL wasCancelled;
@property (assign) int appStatus; /* 0 = green, 1 = orange, 2 = red */
@end

static const char *ALL_VERSIONS[] = {
    "26.3", "26.1.2", "26.1.1", "26.1",
    "1.21.11", "1.21.10", "1.21.8", "1.21.5", "1.21.4", "1.21.3", "1.21.1", "1.21",
    "1.20.6", "1.20.4", "1.20.2", "1.20.1", "1.20",
    "1.19.4", "1.19.2", "1.18.2", "1.17.1", "1.16.5", "1.12.2", "1.8.9", "1.7.10",
    "26w14a", "25w14craftmine", "24w14potato", "23w13a_or_b", "20w14infinite",
    "b1.7.3", "b1.8.1", "a1.2.6", "a1.1.2_01", "c0.30_01c", "rd-132211",
    NULL
};

static BOOL is_snapshot_ver(const char *v) {
    return (strchr(v, 'w') != NULL || strstr(v, "pre") != NULL || strstr(v, "rc") != NULL ||
            strstr(v, "craftmine") != NULL || strstr(v, "potato") != NULL || strstr(v, "infinite") != NULL);
}

static BOOL is_beta_ver(const char *v) {
    return (v == 'b' || v == 'a' || v == 'c' || strncmp(v, "rd-", 3) == 0);
}

static void mkdir_p_mac(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
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
}

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    self.isDark = NO;
    self.isRunning = NO;
    self.appStatus = 0;

    NSRect frame = NSMakeRect(0, 0, 520, 440);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [self.window setTitle:@"PortableMC"];
    [self.window center];

    NSView *content = [self.window contentView];

    /* Header */
    NSTextField *lblTitle = [NSTextField labelWithString:@"PortableMC"];
    [lblTitle setFont:[NSFont boldSystemFontOfSize:20]];
    [lblTitle setFrame:NSMakeRect(22, 396, 130, 28)];
    [content addSubview:lblTitle];

    self.statusDot = [[NSView alloc] initWithFrame:NSMakeRect(142, 405, 10, 10)];
    [self.statusDot setWantsLayer:YES];
    self.statusDot.layer.cornerRadius = 5.0;
    self.statusDot.layer.backgroundColor = [[NSColor systemGreenColor] CGColor];
    [content addSubview:self.statusDot];

    self.btnTheme = [[NSButton alloc] initWithFrame:NSMakeRect(380, 396, 118, 28)];
    [self.btnTheme setTitle:@"Light Theme"];
    [self.btnTheme setBezelStyle:NSBezelStyleRounded];
    [self.btnTheme setTarget:self];
    [self.btnTheme setAction:@selector(toggleTheme:)];
    [content addSubview:self.btnTheme];

    /* Row 1: Auth + User + Login */
    self.popupAuth = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(20, 354, 135, 26) pullsDown:NO];
    [self.popupAuth addItemsWithTitles:@[@"Username", @"Microsoft"]];
    [self.popupAuth setTarget:self];
    [self.popupAuth setAction:@selector(authChanged:)];
    [content addSubview:self.popupAuth];

    self.editUser = [[NSTextField alloc] initWithFrame:NSMakeRect(165, 355, 332, 24)];
    [self.editUser setPlaceholderString:@"Enter offline username..."];
    [self.editUser setBezelStyle:NSTextFieldRoundedBezel];
    [content addSubview:self.editUser];

    self.btnLogin = [[NSButton alloc] initWithFrame:NSMakeRect(422, 353, 76, 28)];
    [self.btnLogin setTitle:@"Login"];
    [self.btnLogin setBezelStyle:NSBezelStyleRounded];
    [self.btnLogin setHidden:YES];
    [self.btnLogin setTarget:self];
    [self.btnLogin setAction:@selector(loginClicked:)];
    [content addSubview:self.btnLogin];

    /* Row 2: Mod Loader */
    NSTextField *lblLoader = [NSTextField labelWithString:@"Mod Loader"];
    [lblLoader setFrame:NSMakeRect(22, 316, 130, 22)];
    [content addSubview:lblLoader];

    self.popupLoader = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(163, 313, 336, 26) pullsDown:NO];
    [self.popupLoader addItemsWithTitles:@[@"Vanilla", @"fabric", @"forge", @"neoforge", @"quilt"]];
    [content addSubview:self.popupLoader];

    /* Row 3: Version + Snapshot + Beta */
    NSTextField *lblVer = [NSTextField labelWithString:@"Version"];
    [lblVer setFrame:NSMakeRect(22, 276, 130, 22)];
    [content addSubview:lblVer];

    self.popupVersion = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(163, 273, 180, 26) pullsDown:NO];
    [content addSubview:self.popupVersion];

    self.chkSnap = [NSButton checkboxWithTitle:@"Snapshot" target:self action:@selector(populateVersions)];
    [self.chkSnap setFrame:NSMakeRect(350, 275, 85, 22)];
    [content addSubview:self.chkSnap];

    self.chkBeta = [NSButton checkboxWithTitle:@"Beta" target:self action:@selector(populateVersions)];
    [self.chkBeta setFrame:NSMakeRect(440, 275, 60, 22)];
    [content addSubview:self.chkBeta];

    [self populateVersions];

    /* Row 4: Game Folder + Browse + Mod + Skin + Cape */
    NSTextField *lblDir = [NSTextField labelWithString:@"Game folder"];
    [lblDir setFrame:NSMakeRect(22, 236, 130, 22)];
    [content addSubview:lblDir];

    NSString *defaultDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Application Support/minecraft"];
    self.editDir = [[NSTextField alloc] initWithFrame:NSMakeRect(165, 235, 108, 24)];
    [self.editDir setStringValue:defaultDir];
    [self.editDir setBezelStyle:NSTextFieldRoundedBezel];
    [content addSubview:self.editDir];

    NSButton *btnBrowse = [[NSButton alloc] initWithFrame:NSMakeRect(276, 233, 56, 28)];
    [btnBrowse setTitle:@"Browse"];
    [btnBrowse setBezelStyle:NSBezelStyleRounded];
    [btnBrowse setTarget:self];
    [btnBrowse setAction:@selector(browseDir:)];
    [content addSubview:btnBrowse];

    NSButton *btnMod = [[NSButton alloc] initWithFrame:NSMakeRect(333, 233, 54, 28)];
    [btnMod setTitle:@"+ Mod"];
    [btnMod setBezelStyle:NSBezelStyleRounded];
    [btnMod setTarget:self];
    [btnMod setAction:@selector(importMod:)];
    [content addSubview:btnMod];

    NSButton *btnSkin = [[NSButton alloc] initWithFrame:NSMakeRect(388, 233, 54, 28)];
    [btnSkin setTitle:@"+ Skin"];
    [btnSkin setBezelStyle:NSBezelStyleRounded];
    [btnSkin setTarget:self];
    [btnSkin setAction:@selector(importSkin:)];
    [content addSubview:btnSkin];

    NSButton *btnCape = [[NSButton alloc] initWithFrame:NSMakeRect(443, 233, 56, 28)];
    [btnCape setTitle:@"+ Cape"];
    [btnCape setBezelStyle:NSBezelStyleRounded];
    [btnCape setTarget:self];
    [btnCape setAction:@selector(importCape:)];
    [content addSubview:btnCape];

    /* Log Area */
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(22, 82, 476, 138)];
    [scroll setHasVerticalScroller:YES];
    [scroll setBorderType:NSBezelBorder];
    self.textLog = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 476, 138)];
    [self.textLog setEditable:NO];
    [self.textLog setFont:[NSFont fontWithName:@"Menlo" size:11.5]];
    [self.textLog setString:@"Ready.\n"];
    [scroll setDocumentView:self.textLog];
    [content addSubview:scroll];

    /* Launch & Progress Bar */
    self.btnLaunch = [[NSButton alloc] initWithFrame:NSMakeRect(22, 22, 476, 44)];
    [self.btnLaunch setTitle:@"LAUNCH MINECRAFT"];
    [self.btnLaunch setBezelStyle:NSBezelStyleRounded];
    [self.btnLaunch setFont:[NSFont boldSystemFontOfSize:14]];
    [self.btnLaunch setTarget:self];
    [self.btnLaunch setAction:@selector(launchClicked:)];
    [content addSubview:self.btnLaunch];

    self.progressBar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(22, 32, 476, 24)];
    [self.progressBar setStyle:NSProgressIndicatorStyleBar];
    [self.progressBar setIndeterminate:YES];
    [self.progressBar setHidden:YES];
    [content addSubview:self.progressBar];

    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)updateStatusDot {
    if (self.appStatus == 0) self.statusDot.layer.backgroundColor = [[NSColor systemGreenColor] CGColor];
    else if (self.appStatus == 1) self.statusDot.layer.backgroundColor = [[NSColor systemOrangeColor] CGColor];
    else self.statusDot.layer.backgroundColor = [[NSColor systemRedColor] CGColor];
}

- (void)appendLog:(NSString *)text {
    dispatch_async(dispatch_get_main_queue(), ^{
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
        [self.editUser setFrame:NSMakeRect(165, 355, 250, 24)];
        [self.editUser setPlaceholderString:@"Optional Microsoft nickname..."];
    } else {
        [self.btnLogin setHidden:YES];
        [self.editUser setFrame:NSMakeRect(165, 355, 332, 24)];
        [self.editUser setPlaceholderString:@"Enter offline username..."];
    }
}

- (void)populateVersions {
    BOOL showSnap = ([self.chkSnap state] == NSControlStateValueOn);
    BOOL showBeta = ([self.chkBeta state] == NSControlStateValueOn);
    [self.popupVersion removeAllItems];

    for (int i = 0; ALL_VERSIONS[i] != NULL; i++) {
        const char *v = ALL_VERSIONS[i];
        if (is_snapshot_ver(v) && !showSnap) continue;
        if (is_beta_ver(v) && !showBeta) continue;
        [self.popupVersion addItemWithTitle:[NSString stringWithUTF8String:v]];
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
- (void)importMod:(id)sender {
    NSString *baseDir = [self.editDir stringValue];
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

- (void)importSkinOrCape:(BOOL)isCape {
    if ([self.popupAuth indexOfSelectedItem] == 1) {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://minecraft.net"]];
        return;
    }

    if ([self.popupLoader indexOfSelectedItem] == 0) {
        [self showAlert:@"Vanilla Not Supported"
                message:@"Offline skins and capes cannot be applied on Vanilla.\n\nPlease select Fabric, Forge, NeoForge, or Quilt and install the CustomSkinLoader mod."
                 folder:nil];
        return;
    }

    NSString *baseDir = [self.editDir stringValue];
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
            [self showAlert:(isCape ? @"Cape Imported" : @"Skin Imported")
                    message:@"Texture saved! Make sure CustomSkinLoader is inside your mods folder."
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

- (void)runCliArgs:(NSArray<NSString *> *)args {
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

    NSFileHandle *fh = [pipe fileHandleForReading];
    fh.readabilityHandler = ^(NSFileHandle *handle) {
        NSData *data = [handle availableData];
        if ([data length] > 0) {
            NSString *str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            if (str) [self appendLog:str];
        }
    };

    __weak AppDelegate *weakSelf = self;
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
                             folder:[weakSelf.editDir stringValue]];
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
    NSString *dir = [self.editDir stringValue];
    mkdir_p_mac([dir UTF8String]);
    [self appendLog:@"Starting Microsoft authentication...\n"];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://microsoft.com"]];
    [self runCliArgs:@[@"--main-dir", dir, @"auth", @"login", @"--no-browser"]];
}

- (void)launchClicked:(id)sender {
    if (self.isRunning && self.activeTask) {
        self.wasCancelled = YES;
        [self.activeTask terminate];
        return;
    }

    NSString *dir = [self.editDir stringValue];
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
    [self runCliArgs:args];
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
