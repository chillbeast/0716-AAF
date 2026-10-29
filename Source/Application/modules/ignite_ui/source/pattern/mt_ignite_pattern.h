/*
 * Copyright (c) - 2022 - MUSIC Tribe, MY.
 * This file is part of IGNITE.
 *
 * IGNITE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * IGNITE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with IGNITE. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once
class AppHome;
namespace ui
{
	class IgnitePatternRow;
	class IgniteStep;
	class AppDnp;
	class StepPlayhead;

	class IgnitePattern :
		public juce::Component,
		public juce::Button::Listener,
		public juce::ComboBox::Listener,
		public CommonLabel::Listener,
		public IgniteControlOptions::Listener,
		public juce::DragAndDropContainer
	{
	public:
		enum class PatternView
		{
			STEP,
			LIST,
			BAR,
			GRID,
			BAR_OPTIONS,
		};

		std::map<juce::String, fpLEDPattern> fpPatternParamLedValues =
		{
			{ "Velocity",   fpLEDPattern::EN_RGB_LED_RED },
			{ "Len:Steps",  fpLEDPattern::EN_RGB_LED_BLUE },
			{ "Len:Tick",   fpLEDPattern::EN_RGB_LED_PURPLE },
			{ "Offset",     fpLEDPattern::EN_RGB_LED_GREEN},
		};


		std::map<FrontPannelParam, FrontPannelParam> fpRGBDispLed =
		{
			{ FrontPannelParam::paramKey_TRACKSEL1, FrontPannelParam::param_RGB_DISP1},
			{ FrontPannelParam::paramKey_TRACKSEL2, FrontPannelParam::param_RGB_DISP2},
			{ FrontPannelParam::paramKey_TRACKSEL3, FrontPannelParam::param_RGB_DISP3},
			{ FrontPannelParam::paramKey_TRACKSEL4, FrontPannelParam::param_RGB_DISP4},
			{ FrontPannelParam::paramKey_TRACKSEL5, FrontPannelParam::param_RGB_DISP5},
			{ FrontPannelParam::paramKey_TRACKSEL6, FrontPannelParam::param_RGB_DISP6},
			{ FrontPannelParam::paramKey_TRACKSEL7, FrontPannelParam::param_RGB_DISP7},
			{ FrontPannelParam::paramKey_TRACKSEL8, FrontPannelParam::param_RGB_DISP8},
			{ FrontPannelParam::paramKey_TRACKSEL9, FrontPannelParam::param_RGB_DISP9},
			{ FrontPannelParam::paramKey_TRACKSEL10, FrontPannelParam::param_RGB_DISP10},
			{ FrontPannelParam::paramKey_TRACKSEL11, FrontPannelParam::param_RGB_DISP11},
			{ FrontPannelParam::paramKey_TRACKSEL12, FrontPannelParam::param_RGB_DISP12},
			{ FrontPannelParam::paramKey_TRACKSEL13, FrontPannelParam::param_RGB_DISP13},
			{ FrontPannelParam::paramKey_TRACKSEL14, FrontPannelParam::param_RGB_DISP14},
			{ FrontPannelParam::paramKey_TRACKSEL15, FrontPannelParam::param_RGB_DISP15},
			{ FrontPannelParam::paramKey_TRACKSEL16, FrontPannelParam::param_RGB_DISP16},
		};

		std::function<void(int)> onPlayHeadChanged;
		struct NodeHash
		{
			std::size_t operator()(const mt::Ignite::selectedNode& n) const
			{
				return std::hash<int>()(n.note) ^ (std::hash<int>()(n.step) << 1);
			}
		};
		IgnitePattern();
		~IgnitePattern() override;

		void refreshPatternUI();
		void refreshStatusbar();

		void addPattern(mt::Ignite::patternData* data, bool isPatternUpdateneeded, bool isPatternCreatedInTrack = false);
		void addBar(mt::Ignite::patternData* data, int numBars);
		void addStep(ClienPatterntBarData* data, int numSteps);
		void addNote(mt::Ignite::patternData* data, int numNotes);
		void addNextNote(int barIndex, int stepIndex);
		void addNote(int barIndex, int stepNum, int noteNum);
		void addNotes(int sMin, int sMax, int nMin, int nMax);
		void addStepEvent(int barIndex, int stepIndex);

		void delStepEvent(int eventType, int barIndex, int stepIndex);
		bool delNote(int barIndex, int stepNum, int noteNum, bool isUpdateEngine = false);
		void delNotes(int sMin, int sMax, int nMin, int nMax);
		void deleteAllUnassignedPatterns();

		void requestDataFromEngine(int index);

		void updatePatternData(uint8_t trackBarId, uint8_t trackPlayStepId);
		void updatePatternData(uint8_t param, const std::string& value);
		void updateLastStep(int stepId);
		void updateDataFromEngine();
		void updateAccidentals(bool state);
		void updatePatternEditIndication();

		void setSelected(int index);
		void setEditPattern(int index) { editPattern = index; }
		void setDefaultPdata(mt::Ignite::patternData* data, mt::Ignite::ptype type);
		void setPatternData(mt::Ignite::ptDataIndex item_id, String data, bool is_internal);
		void setPatternData(mt::Ignite::ptDataIndex item_id, uint8_t data, bool is_internal);
		void setPatternData(mt::Ignite::ptDataIndex item_id, int16_t data, bool is_internal);
		void setBarData(mt::Ignite::barDataIndex item_id, int index, uint8_t data, bool is_internal);
		void setStepData(mt::Ignite::stepDataIndex item_id, int step_index, int bar_index, uint8_t data, bool is_internal);
		void setStepEventData(mt::Ignite::stepEventDataIndex item_id, int bar_index, int step_index, int index, uint8_t data, bool is_internal);
		void setNodeData(mt::Ignite::nodeDataIndex item_id, int barIndex, int stepIndex, int nodeId, int16_t data, bool is_internal, bool isPlayModeData = false);
		void setPresetValueForSteps(mt::Ignite::nodeDataIndex item_id, std::vector<int> values, bool, bool isPlayModeData = false);
		void setPresetValueForSteps(mt::Ignite::nodeDataIndex item_id, int note, std::vector<int> values, bool, bool isPlayModeData = false);
		void setDataForSelectedNode(mt::Ignite::nodeDataIndex item_id, int stepId, int16_t data, bool is_internal, bool isPlayModeData = false);
		void setSelectedNotesValue(mt::Ignite::nodeDataIndex item_id, int value, bool isPlayModeData);
		void setSelectedNotesValue(mt::Ignite::nodeDataIndex item_id, bool isPlayModeData);
		void setActiveNode(uint8_t noteNum);
		void setPatternIndex(int index);
		void setEditTrackPattern(String pt_name);
		void setStepIndex(int index);
		void setPatternEditIndication(bool state);
		void setThreshold(int value) { paramThreshold = value; }
		void setGangStep(int step_index, bool state);
		void setGangNote(int note_index, bool state) { selectedNotes[note_index] = state; }
		void setSelectedNote(int step, int note);
		void setSelectedNotes(int start_step, int end_step, int start_note, int end_note);
		void setGangMode(bool state);
		void setSelectedNoteForAllSteps(int note);
		void setNotesForSelectedStep(int note, bool state);
		void setActiveParamControl(juce::String param) { sparamListValue = param; }
		void setPatternDataUpdateStatus(bool state) { patternDataUpdated = state; }
		void setLastKeyStatus(bool state) { lastKeyStatus = state; }
		void setTieKeyStatus(bool state) { tieKeyStatus = state; }
		void setTransposeKeyStatus(bool state) { transposeKeyStatus = state; }
		void setSkipKeyStatus(bool state) { skipKeyStatus = state; }
		void setPatternMoveState(bool state) { moveEnabled = state; }
		void setPasteCompletionState(bool state) { pasteCompleted = state; }
		bool getLastKeyStatus() { return lastKeyStatus; }
		bool getInsertKeyStatus() { return insertKeyStatus; }
		void setInsertKeyStatus(bool state) { insertKeyStatus = state; }
		void setGangKeyStatus(bool state) { gangKeyStatus = state; }
		void resetInsertStatus();
		void resetGangStatus();
		void setFpRotaryLedState();

		int getSelected(void);
		mt::Ignite::patternData* getPatternData(uint8_t index);
		void getNextNodeAndUpdate(bool direction);
		void getNextStepAndUpdate(bool direction);
		void getNextBarAndUpdate(bool direction);
		void addStepNoteIfNotPresent(int stepNum);
		bool isNotePresent(int stepNum, int noteNum);
		var getPatternData(mt::Ignite::ptDataIndex item_id);
		var getBarData(mt::Ignite::barDataIndex item_id, int index);
		var getStepData(mt::Ignite::stepDataIndex item_id, int bar_index, int step_index);
		var getStepEventData(mt::Ignite::stepEventDataIndex item_id, int bar_index, int step_index, int index);
		var getNodeData(mt::Ignite::nodeDataIndex item_id, int barIndex, int stepIndex, int nodeIndex);
		var getDataForSelectedNode(mt::Ignite::nodeDataIndex item_id, int stepId, int nodeId);
		ClientPatternStepData* getStepData(int barIndex, int stepIndex);
		int getActiveStepIndex();
		uint16_t getCurrentStepPlaying();
		ClientPatternNodeData getSingleNote();
		uint8_t getNodeCount(int stepNum);
		int getThreshold() { return paramThreshold; }
		int getStepId(int stepId);
		int getTabIndex();
		int getCurrentNode();
		int getCurrentBarStepId(int step_id);
		int getCurrentBarId();
		juce::String getActiveParamControl() { return sparamListValue; }
		int getValueForSliderPos(mt::Ignite::nodeDataIndex item_id);
		bool getGangMode() { return isGangModeEnabled; }
		bool* getGangStepsState() { return selectedStepInGang; }
		bool getGangStepState(int step_index);
		bool* getGangNotesState() { return selectedNotes; }
		bool getGangNoteState(int note_index) { return selectedNotes[note_index]; }
		bool getSelectedNote(mt::Ignite::selectedNode node);
		std::unordered_set<mt::Ignite::selectedNode, NodeHash>* getSelectedNodes() { return &selectedNotesInGrid; }
		std::vector<mt::Ignite::selectedNode>* getSelectedNodesVec() { return &selectedNotesInGridVec; }
		bool getNoteCutPasteState() { return noteCutPasteActive; }
		bool getPatternDataUpdateStatus() { return patternDataUpdated; }
		bool getTieKeyStatus() { return tieKeyStatus; }
		bool getTransposeKeyStatus() { return transposeKeyStatus; }
		bool getSkipKeyStatus() { return skipKeyStatus; }
		bool getPatternMoveState() { return moveEnabled; }
		bool getGangKeyStatus() { return gangKeyStatus; }
		bool getAccidentalEnabledState() { return isAccidentalEnabled; }
		bool isPatternCreated(void) { return(patternCreated); }
		bool isInitCompleted(void) { return(initCompleted); }

		void notesCutPaste(int stepNum, int noteNum);
		void notesMove(bool rowRotary, bool direction);

		void pasteSelectedNotes(int stepNum, int noteNum, bool isMoveOp = false);
		void movePattern(juce::Point<float> position);
		void movePattern(int destIndex);

		void changePatternViewVisibility(PatternView viewToShow, bool isPatternCreatedInTrack = false);
		void enableControlButton(IgniteControlOptions::eKey key, bool state) { _pPatternOptCtl->enableKey(key, state); };
		void parameterViewDrag(int selected_step, int velocity, mt::Ignite::nodeDataIndex id);

		void rotatePattern(uint16_t barId, uint16_t stepId);

		void insertBar(IgniteBarInsert::insertCtlKey key, int barNumber);
		void copyPatternBar(IgniteBarCopy::copyBarCtlKey key, int fromBarId, int toBarId);
		void savePattern();
		bool insertBlankStep();

		void updateLables();
		void resetNoteSelection();
		void resetGangStepState();
		void resetGangNoteState();
		void resetSelectedNoteRefCount();
		bool pannelKeyHandler(paramId_t param, uint32_t KeyValue);
		void onOptCtlButtonClicked(IgniteControlOptions::eKey option) override;
		void setFpStepLedState();
		void setFpPlayHeadLedState(int playStepNumber);
		void setFpPatternLedState();
		void setCurrentStepLedState();
		void setPatternStepModes();
		void setPatternBarViewModes();
		void setFpStepGateLedState();
		bool handleTrackSelection(paramId_t param, uint32_t KeyValue);
		int getPatternCurrentBank();
		void getSelectedBarAndUpdate(int barNumber);
		void updateBarData();
		void updateSelectedBarData();
		void showPatternBars();
		void setFpModeLedState();
		bool handleLastKey(paramId_t param, uint32_t KeyValue);
		bool handlePatternLastStep(paramId_t param, uint32_t KeyValue);
		void setFpPatternLastStepLedState();

		static IgnitePattern* getAppPatternInstance()
		{
			static IgnitePattern* getPatternInstance;
			if (NULL == getPatternInstance)
			{
				getPatternInstance = new IgnitePattern();
			}

			return getPatternInstance;
		}

		std::vector<mt::Ignite::selectedNode> selectedNotesInGridVec;
		std::unordered_set<mt::Ignite::selectedNode, NodeHash> selectedNotesInGrid;

		CommonPopUp* _pPopUpPtr = nullptr;
		IgniteLoopControl* _pBarLoopComp;
		CommonInfoWindow* _pInfoWindow = nullptr;
		bool isGlobalAutosaveEnabled() { return isAutosave; };
		VariableMinMax getTickRange() { return list[selected]->getLengthRange(); };
		VariableMinMax getDelayRange() { return list[selected]->getOffsetRange(); };
		void updateTickDelayRange() {
			_pComponentStep->updateLegnthDelay();
		};
		void setSulptKeyStatus(bool state) { _pComponentStep->setSulptKeyStatus(state); }
		bool getSculptKeyStatus() { return _pComponentStep->getSulptKeyStatus(); }
		int getLastStepIndex() { return list[selected]->getLastStepIndex();; }
	private:
#define PATTERN_OPTIONS_VIEW 9
		enum optionsViewItemID
		{
			NO_FOCUS = 0,
			PATTERN_MOVE_DOWN,
			PATTERN_MOVE_UP,
			PATTERN_MOVE_TOP,
			PATTERN_MOVE_LAST,
			PATTERN_MOVE,
			PATTERN_UNDO,
			PATTERN_COPY,
			PATTERN_CONFIG,
			FOCUS_MAX = PATTERN_CONFIG
		};

		enum PATTERN_FOCUS
		{
			PATTERN_DETAIL,
			PATTERN_LIST,
			PATTERN_OPTION,
		};

		CommonWarningPopUp* _pWarnPopupPtr = nullptr;
		IgniteStep* _pStepComp = nullptr;
		StepSequencer* _pComponentStep = nullptr;
		AppGrid* _pComponentGrid = nullptr;
		AppDnp* _pEmptyPatternComp = nullptr;
		PatternStatusBar* _pPatternStatusBarPtr = nullptr;
		AppMainmode* _pMainmodeptr = nullptr;
		IgniteControlOptions* _pPatternOptCtl = nullptr;
		AppOptions* _pAppOptPtr = nullptr;
		StepPlayhead* _pPlayHead = nullptr;
		IgniteSongConfig* _pSongCfgPtr = nullptr;
		IgniteCopy* _pPatternCopyComp = nullptr;
		IgniteScene* scenePtr = nullptr;

		uint8_t optionsView[PATTERN_OPTIONS_VIEW] = { NO_FOCUS, PATTERN_MOVE_DOWN, PATTERN_MOVE_UP, PATTERN_MOVE_TOP, PATTERN_MOVE_LAST, PATTERN_MOVE, PATTERN_UNDO, PATTERN_COPY, PATTERN_CONFIG };
		uint8_t optionHandlerEnabled = PATTERN_DETAIL;
		PatternView currentView = PatternView::LIST;
		paramId_t itemParam;
		uint32_t itemKeyValue;
		uint16_t currentStepPlaying = 0;

		bool itemFocus = false;
		bool initCompleted = false;
		bool bootupState = true;
		bool patternDataUpdated = false;
		bool moveEnabled = false;
		bool patternCreated = false;
		bool isGangModeEnabled = false;
		bool selectedStepInGang[256]{ false };
		bool selectedNotes[128]{ false };
		bool select[16][128] = { {false} };
		bool isAccidentalEnabled = false;
		bool gangKeyStatus = false;
		bool tieKeyStatus = false;
		bool transposeKeyStatus = false;
		bool skipKeyStatus = false;
		bool insertKeyStatus = false;
		bool lastKeyStatus = false;
		bool pasteCompleted = false;
		bool addNewInListHiglited = false;
		bool ledFillTransposeSlowBlink = false;
		bool ledFillTransposeFastBlink = false;
		bool ledSoloSkipSlowBlink = false;
		bool ledSoloSkipFastBlink = false;
		bool noteCutPasteActive = false;
		int previousBarNumber = 0;
		mt::Ignite::operatioMode opMode = mt::Ignite::operatioMode::IDLE_MODE;

		int selected{ 0 };
		int editPattern{ 0 };
		int prevSelected{ 0 };
		int patternIndex = 0;
		int defaultNote = 48;
		int paramThreshold = 127;
		int patternRowHighLightIndex = 0;
		int stepCountMax = 0;
		int noteCountMax = 0;
		int curretPatternRow = 0;
		int keyFocusedItem = optionsViewItemID::NO_FOCUS;
		int previousStepNumber = -1;
		int currentPatternBank = 0;

		juce::OwnedArray<IgnitePatternRow> list;
		std::vector<juce::String> barList{ "No Bar Selected" };
		std::list <ClientPatternNodeData> nodeListData;

		CommonNameLabel patternName{ "PATTERN 1" };
		CommonNameLabel patternListNameLable{ "NAME" };
		CommonNameLabel patternDirectionLable{ "DIRECTION" };
		CommonNameLabel patternNumBarLable{ "NUMBER OF BARS" };
		CommonNameLabel patternTimebaseLable{ "TIMEBASE" };
		CommonNameLabel patternLastStepLable{ "LAST STEP" };
		CommonNameLabel patternRotateLable{ "ROTATE PATTERN" };

		CommonButton addNewP{ "" };
		CommonImageButton addNewInList{ "+ Add New Pattern", "btn-add-newpattern-default.png", "btn-add-newpattern-active.png", "btn-add-newpattern-hover.png" };
		CommonButton addNewM{ " Create Mono Pattern " };
		CommonToggleButton exapndedView{ "Expand" };
		CommonImageButton barView{ "Bar  View","btn-pattern-bar-default.png","btn-pattern-bar-active.png","btn-pattern-bar-default-hover.png" };
		CommonImageButton listView{ "List View","btn-song-list-default.png","btn-song-list-active.png","btn-song-list-default-hover.png" };
		CommonImageButton stepView{ "Step View","btn-pattern-step-default.png","btn-pattern-step-active.png","btn-pattern-step-default-hover.png" };
		CommonImageButton gridView{ "Grid View","btn-pattern-grid-default.png","btn-pattern-grid-active.png","btn-pattern-grid-default-hover.png" };
		juce::FlexBox patternBox;
		juce::FlexBox patternLableBox;
		juce::Viewport view;
		juce::Component container;
		juce::String sparamListValue = "Velocity";
		bool isAutosave = false;

		void cleanup();
		void populate(mt::Ignite::patternData* data, bool isPatternUpdateneeded, int patternIndex = -1);
		void paint(juce::Graphics& g) override;
		void resized() override;
		void buttonClicked(juce::Button* buttonThatWasClicked) override;
		void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
		void labelTextChanged(Label* labelThatHasChanged) override;
		/*void updatePattern(int num_of_steps);*/
		String getPatternName();
		void updateBarList();
		void showPopup(int index);
		void enableViewBox(PatternView state);
		void setLoopControl(IgniteLoopControl::loopKey key, int loopStartIndex, int loopEndIndex);
		void copyPattern(IgniteCopy::copyCtlKey key, String copyName);
		void refreshBarLoop();
		void setCurrentStepPlaying(uint16_t step) { currentStepPlaying = step; };
		void resetPatterView();
		void refreshStepUI(ClientPatternStepData* data);
		void cutNote(int stepNum, int noteNum);
		void copyNote(int stepNum, int noteNum);
		void clearCutCopy();
		void pasteSingleNote(int step, int note, ClientPatternNodeData nData);
		void handleCommandMessage(int id);
		bool pannelKeyOptionsHandler(paramId_t param, uint32_t keyValue);
		void pannelSendKey(bool isFocuse, paramId_t param, uint32_t keyValue);
		bool handleTie(paramId_t param, uint32_t KeyValue);
		bool handleTranspose(paramId_t param, uint32_t KeyValue);
		bool handleSkip(paramId_t param, uint32_t KeyValue);
		void ctlPatternListHeader(bool state);
		void setFpStepTieLedState();
		void setFpStepTransposeLedState();
		void setFpStepSkipLedState();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IgnitePattern)
	};
};
