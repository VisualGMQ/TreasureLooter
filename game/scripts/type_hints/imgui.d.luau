--!strict
-- luau-lsp definition file for the `ImGui` C++ binding namespace.
-- Generated equivalent of common/src/scripts/script_imgui_binding.cpp.

-- The runtime global table created by `beginNamespace("ImGui")`.
declare ImGui: {
	-- Window
	Begin: (name: string) -> boolean,
	End: () -> (),
	BeginChild: (str_id: string, w: number?, h: number?, child_flags: number?) -> boolean,
	EndChild: () -> (),

	-- Text
	Text: (text: string) -> (),
	SeparatorText: (label: string) -> (),

	-- Main widgets
	Button: (label: string) -> boolean,
	Checkbox: (label: string, v: boolean) -> (boolean, boolean),
	InputText: (label: string, initial: string) -> (string, boolean),
	InputInt: (label: string, v: number) -> (number, boolean),
	InputFloat: (label: string, v: number) -> (number, boolean),
	DragFloat: (label: string, v: number, speed: number?, v_min: number?, v_max: number?) -> (number, boolean),
	DragInt: (label: string, v: number, speed: number?, v_min: number?, v_max: number?) -> (number, boolean),
	SliderFloat: (label: string, v: number, v_min: number, v_max: number) -> (number, boolean),
	SliderInt: (label: string, v: number, v_min: number, v_max: number) -> (number, boolean),

	-- Tree
	TreeNode: (label: string) -> boolean,
	TreeNodeEx: (label: string, flags: number) -> boolean,
	TreePop: () -> (),
	CollapsingHeader: (label: string) -> boolean,

	-- Layout
	SameLine: () -> (),
	Separator: () -> (),
	NewLine: () -> (),
	Spacing: () -> (),
	Indent: () -> (),
	Unindent: () -> (),

	-- ID
	PushID: (id: number | string) -> (),
	PopID: () -> (),

	-- Disable
	BeginDisabled: (disabled: boolean?) -> (),
	EndDisabled: () -> (),

	-- Popup
	BeginPopup: (str_id: string) -> boolean,
	EndPopup: () -> (),
	OpenPopup: (str_id: string) -> (),
	CloseCurrentPopup: () -> (),

	-- Tooltip
	BeginTooltip: () -> boolean,
	EndTooltip: () -> (),
	SetTooltip: (fmt: string) -> (),

	-- Item query
	IsItemHovered: () -> boolean,
	IsItemClicked: () -> boolean,

	-- Enum / flag constants
	WindowFlags_None: number,
	WindowFlags_NoTitleBar: number,
	WindowFlags_NoResize: number,
	WindowFlags_NoMove: number,
	WindowFlags_NoScrollbar: number,
	WindowFlags_NoCollapse: number,
	WindowFlags_AlwaysAutoResize: number,
	WindowFlags_NoBackground: number,
	WindowFlags_NoSavedSettings: number,
	WindowFlags_MenuBar: number,
	WindowFlags_NoDecoration: number,
	WindowFlags_NoInputs: number,

	Cond_None: number,
	Cond_Always: number,
	Cond_Once: number,
	Cond_FirstUseEver: number,
	Cond_Appearing: number,

	TreeNodeFlags_None: number,
	TreeNodeFlags_Selected: number,
	TreeNodeFlags_Framed: number,
	TreeNodeFlags_DefaultOpen: number,
	TreeNodeFlags_OpenOnDoubleClick: number,
	TreeNodeFlags_Leaf: number,
	TreeNodeFlags_OpenOnArrow: number,
	TreeNodeFlags_Bullet: number,
	TreeNodeFlags_CollapsingHeader: number,

	InputTextFlags_None: number,
	InputTextFlags_CharsDecimal: number,
	InputTextFlags_CharsHexadecimal: number,
	InputTextFlags_ReadOnly: number,
	InputTextFlags_Password: number,
	InputTextFlags_AutoSelectAll: number,

	SliderFlags_None: number,
	SliderFlags_Logarithmic: number,
	SliderFlags_NoRoundToFormat: number,
	SliderFlags_NoInput: number,
	SliderFlags_AlwaysClamp: number,

	PopupFlags_None: number,
	PopupFlags_MouseButtonLeft: number,
	PopupFlags_MouseButtonRight: number,
	PopupFlags_MouseButtonMiddle: number,
	PopupFlags_NoReopen: number,
	PopupFlags_NoOpenOverExistingPopup: number,

	HoveredFlags_None: number,
	HoveredFlags_ChildWindows: number,
	HoveredFlags_RootWindow: number,
	HoveredFlags_AnyWindow: number,
	HoveredFlags_AllowWhenBlockedByPopup: number,
	HoveredFlags_AllowWhenDisabled: number,
	HoveredFlags_ForTooltip: number,

	FocusedFlags_None: number,
	FocusedFlags_ChildWindows: number,
	FocusedFlags_RootWindow: number,
	FocusedFlags_AnyWindow: number,

	SelectableFlags_None: number,
	SelectableFlags_NoAutoClosePopups: number,
	SelectableFlags_SpanAllColumns: number,
	SelectableFlags_AllowDoubleClick: number,
	SelectableFlags_Disabled: number,

	ComboFlags_None: number,
	ComboFlags_PopupAlignLeft: number,
	ComboFlags_HeightSmall: number,
	ComboFlags_HeightRegular: number,
	ComboFlags_HeightLarge: number,
	ComboFlags_NoArrowButton: number,

	ChildFlags_None: number,
	ChildFlags_Borders: number,
	ChildFlags_FrameStyle: number,

	Dir_None: number,
	Dir_Left: number,
	Dir_Right: number,
	Dir_Up: number,
	Dir_Down: number,

	MouseButton_Left: number,
	MouseButton_Right: number,
	MouseButton_Middle: number,
}
