/* WNCampaign */

#import <Cocoa/Cocoa.h>
#import "WMLCampaign.h"
#import "WMLMap.h"
#import "WNCampaignIconView.h"
#import "WNLoadSave.h"

@interface WNCampaign : NSObject
{
    IBOutlet id agressionLevel;
    IBOutlet id campaignID;
    IBOutlet NSTextField *campaignName;
    IBOutlet NSTextField *campaignPrefix;
    IBOutlet NSButton *canRecruit;
	IBOutlet WNCampaignIconView *campaignIcon;
    IBOutlet id defeatList;
    IBOutlet NSTextField *difficultyDescEasy;
    IBOutlet NSTextField *difficultyDescNormal;
    IBOutlet NSTextField *difficultyDescHard;
    IBOutlet id difficultyEasy;
    IBOutlet WNCampaignIconView *difficultyIconEasy;
    IBOutlet id difficultyHard;
    IBOutlet WNCampaignIconView *difficultyIconHard;
    IBOutlet id difficultyMedium;
    IBOutlet WNCampaignIconView *difficultyIconNormal;
    IBOutlet NSTextField *goldAmount;
    IBOutlet NSTableView *keepSide;
    IBOutlet NSTextField *keepX;
    IBOutlet WNLoadSave *loadSave;
    IBOutlet NSTextField *keepY;
    IBOutlet id leaderList;
    IBOutlet id recruitList;
    IBOutlet NSTabView *mainTabView;
    IBOutlet id mapHeight;
    IBOutlet id mapWidth;
	IBOutlet NSTextField *mapInfoTerrain;
	IBOutlet NSTextView *mapInfoCharacterList;
	IBOutlet NSTextView *mapInfoUnitList;
    IBOutlet NSPopUpButton *mapUnitSideMenu;
    IBOutlet NSSlider *mapZoom;
    IBOutlet NSTableView *nextScenarioVictory;
    IBOutlet NSTableView *nextScenarioContinue;
    IBOutlet NSTableView *nextScenarioDefeat;
    IBOutlet NSTextField *noTurns;
	IBOutlet NSTextView *scenarioAdvancedInfo;
    IBOutlet id scenarioList;
    IBOutlet NSPopUpButton *showConditionsForDifficulty;
	IBOutlet NSTextView *sidesAdvancedInfo;
    IBOutlet NSPopUpButton *sidesShowDetailForDifficulty;
    IBOutlet id sidesList;
    IBOutlet NSTableView *teamList;
    IBOutlet id terrainList;
    IBOutlet id typeAI;
    IBOutlet id typeHuman;
    IBOutlet id unitList;
    IBOutlet NSButton *unitPlacementEasy;
    IBOutlet NSButton *unitPlacementHard;
    IBOutlet NSButton *unitPlacementNormal;
    IBOutlet id victoryList;
}
- (IBAction)aggressionSelect:(id)sender;
- (IBAction)campaignIDSet:(id)sender;
- (IBAction)campaignNameSet:(id)sender;
- (IBAction)campaignPrefixSet:(id)sender;
- (IBAction)canRecruit:(id)sender;
- (IBAction)defeatAdd:(id)sender;
- (IBAction)defeatRemove:(id)sender;
- (IBAction)defeatSelect:(id)sender;
- (IBAction)difficultyDescChange:(id)sender;
- (IBAction)difficultyEasyCheck:(id)sender;
- (IBAction)difficultyHardCheck:(id)sender;
- (IBAction)difficultyMediumCheck:(id)sender;
- (IBAction)doNothing:(id)sender;
- (IBAction)goldSet:(id)sender;
- (IBAction)keepCancel:(id)sender;
- (IBAction)keepChoose:(id)sender;
- (IBAction)leaderSelect:(id)sender;
- (IBAction)mapCharacterSelect:(id)sender;
- (IBAction)mapExport:(id)sender;
- (IBAction)mapHeightSet:(id)sender;
- (IBAction)mapImport:(id)sender;
- (IBAction)mapUnitSide:(id)sender;
- (IBAction)mapViewSliderSet:(id)sender;
- (IBAction)mapWidthSet:(id)sender;
- (IBAction)mapZoomSet:(id)sender;
- (IBAction)nextScenarioVictorySelect:(id)sender;
- (IBAction)nextScenarioContinueSelect:(id)sender;
- (IBAction)noTurnsSet:(id)sender;
- (IBAction)recruitSelect:(id)sender;
- (IBAction)scenarioDelete:(id)sender;
- (IBAction)scenarioNew:(id)sender;
- (IBAction)scenarioSelect:(id)sender;
- (IBAction)showConditionsForDifficulty:(id)sender;
- (IBAction)sidesAdd:(id)sender;
- (void)sidesChanged:(NSNotification *)notification;
- (IBAction)sidesRemove:(id)sender;
- (IBAction)sidesShowDetailFor:(id)sender;
- (void)selectSide: (int)index;
- (IBAction)sidesSelect:(id)sender;
- (void)populateScenarioInfo;
- (void)populateSidesData;
- (IBAction)teamAdd:(id)sender;
- (IBAction)teamRemove:(id)sender;
- (IBAction)teamSelect:(id)sender;
- (IBAction)terrainSelect:(id)sender;
- (IBAction)typeAI:(id)sender;
- (IBAction)typeHuman:(id)sender;
- (IBAction)unitPlacementSetEasy:(id)sender;
- (IBAction)unitPlacementSetHard:(id)sender;
- (IBAction)unitPlacementSetNormal:(id)sender;
- (IBAction)unitSelect:(id)sender;
- (IBAction)victoryAdd:(id)sender;
- (IBAction)victoryRemove:(id)sender;
- (IBAction)victorySelect:(id)sender;
-(void)campaignReset:(NSNotification *)notification;
-(void)awakeFromNib;
-(void)doInit;
+(WMLCampaign *)getMainCampaign;
+(void)setMainCampaign:(WMLCampaign *)newCampaign;
-(void)selectScenario: (int) scenarioIndex;
+(WMLScenario *)getActiveScenario;
+(WMLMap *)getActiveMap;
-(WMLSideDetail *)getActiveSideDetail;
+(float)getMapZoom;
+(NSString *)getDifficulty;
+(WMLScenarioDetails *)getActiveScenarioDetail;
+(NSWindow *)windowForTitle:(NSString *)titleToFind;
-(void)newCampaignIcon:(NSNotification *)notification;
-(void)textDidChange:(NSNotification *)notification;
+(WMLTag *)getEventTags;
+(WMLTag *)getEventTagForTag:(NSString *)tag;
+(void)setMainMenu:(NSMenu *)newMainMenu;
+(NSMenu *)getMainMenu;
+(void)setEditorMenu:(NSMenu *)newEditorMenu;
+(NSMenu *)getEditorMenu;
+(void)switchToMainMenu;
+(void)switchToEditorMenu;
-(void)showEditorWindow;
-(NSWindow *)getEditorWindow;
@end
