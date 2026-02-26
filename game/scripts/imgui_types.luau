export type Begin = (name: string) -> boolean
export type End = () -> ()

export type Text = (text: string) -> ()
export type SeparatorText = (label: string) -> ()

export type Button = (label: string) -> boolean
export type Checkbox = (label: string, v: boolean) -> (boolean, boolean)
export type InputText = (label: string, initial: string) -> (string, boolean)
export type InputInt = (label: string, v: number) -> (number, boolean)
export type InputFloat = (label: string, v: number) -> (number, boolean)
export type DragFloat = (label: string, v: number, speed: number?, v_min: number?, v_max: number?) -> (number, boolean)
export type DragInt = (label: string, v: number, speed: number?, v_min: number?, v_max: number?) -> (number, boolean)
export type SliderFloat = (label: string, v: number, v_min: number, v_max: number) -> (number, boolean)
export type SliderInt = (label: string, v: number, v_min: number, v_max: number) -> (number, boolean)

export type TreeNode = (label: string) -> boolean
export type TreePop = () -> ()
export type CollapsingHeader = (label: string) -> boolean

export type SameLine = () -> ()
export type Separator = () -> ()
export type NewLine = () -> ()
export type Spacing = () -> ()
export type Indent = () -> ()
export type Unindent = () -> ()

export type PushID = (id: number | string) -> ()
export type PopID = () -> ()

export type BeginDisabled = (disabled: boolean?) -> ()
export type EndDisabled = () -> ()

export type BeginPopup = (str_id: string) -> boolean
export type EndPopup = () -> ()
export type OpenPopup = (str_id: string) -> ()
export type CloseCurrentPopup = () -> ()

export type BeginTooltip = () -> boolean
export type EndTooltip = () -> ()
export type SetTooltip = (fmt: string) -> ()

export type IsItemHovered = () -> boolean
export type IsItemClicked = () -> boolean

export type WindowFlags_None = number
export type WindowFlags_NoTitleBar = number
export type WindowFlags_NoResize = number
export type WindowFlags_NoMove = number
export type WindowFlags_NoScrollbar = number
export type WindowFlags_NoCollapse = number
export type WindowFlags_AlwaysAutoResize = number
export type WindowFlags_NoBackground = number
export type WindowFlags_NoSavedSettings = number
export type WindowFlags_MenuBar = number
export type WindowFlags_NoDecoration = number
export type WindowFlags_NoInputs = number

export type Cond_None = number
export type Cond_Always = number
export type Cond_Once = number
export type Cond_FirstUseEver = number
export type Cond_Appearing = number

export type TreeNodeFlags_None = number
export type TreeNodeFlags_Selected = number
export type TreeNodeFlags_Framed = number
export type TreeNodeFlags_DefaultOpen = number
export type TreeNodeFlags_Leaf = number
export type TreeNodeFlags_Bullet = number
export type TreeNodeFlags_CollapsingHeader = number

export type InputTextFlags_None = number
export type InputTextFlags_CharsDecimal = number
export type InputTextFlags_CharsHexadecimal = number
export type InputTextFlags_ReadOnly = number
export type InputTextFlags_Password = number
export type InputTextFlags_AutoSelectAll = number

export type SliderFlags_None = number
export type SliderFlags_Logarithmic = number
export type SliderFlags_NoRoundToFormat = number
export type SliderFlags_NoInput = number
export type SliderFlags_AlwaysClamp = number

export type PopupFlags_None = number
export type PopupFlags_MouseButtonLeft = number
export type PopupFlags_MouseButtonRight = number
export type PopupFlags_MouseButtonMiddle = number
export type PopupFlags_NoReopen = number
export type PopupFlags_NoOpenOverExistingPopup = number

export type HoveredFlags_None = number
export type HoveredFlags_ChildWindows = number
export type HoveredFlags_RootWindow = number
export type HoveredFlags_AnyWindow = number
export type HoveredFlags_AllowWhenBlockedByPopup = number
export type HoveredFlags_AllowWhenDisabled = number
export type HoveredFlags_ForTooltip = number

export type FocusedFlags_None = number
export type FocusedFlags_ChildWindows = number
export type FocusedFlags_RootWindow = number
export type FocusedFlags_AnyWindow = number

export type SelectableFlags_None = number
export type SelectableFlags_NoAutoClosePopups = number
export type SelectableFlags_SpanAllColumns = number
export type SelectableFlags_AllowDoubleClick = number
export type SelectableFlags_Disabled = number

export type ComboFlags_None = number
export type ComboFlags_PopupAlignLeft = number
export type ComboFlags_HeightSmall = number
export type ComboFlags_HeightRegular = number
export type ComboFlags_HeightLarge = number
export type ComboFlags_NoArrowButton = number

export type ChildFlags_None = number
export type ChildFlags_Borders = number
export type ChildFlags_FrameStyle = number

export type Dir_None = number
export type Dir_Left = number
export type Dir_Right = number
export type Dir_Up = number
export type Dir_Down = number

export type MouseButton_Left = number
export type MouseButton_Right = number
export type MouseButton_Middle = number

return ImGui