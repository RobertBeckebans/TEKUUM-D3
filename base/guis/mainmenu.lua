--
-- Example of the main menu GUI in Lua with functions to create design templates like CreateMenuButton
--

print( "package.path: " .. package.path .. "\n" )
--print( "package.cpath: " .. package.cpath .. "\n" )
--print( "package.config: " .. package.config .. "\n" )

--print( "package.loaded: " .. package.loaded .. "\n" )
--print( "package.preload: " .. package.preload .. "\n" )


--local socket = require( "socket" )
--require( 'mobdebug' ).start()

--function CreateDesktopTitle( desktopWindow )
  
--end

GlobalTable = {}
function GlobalTable.TestFunc()
	print( "GlobalTable.TestFunc()" )
end

function TestFunc( window )
	print( "TestFunc: window.text = '" .. window.text .. "'" )
end


MENU_BUTTON_BACK_COLOR = Vec4.new( 0, 0, 0, 0.7 )
MENU_BUTTON_FORE_COLOR = Vec4.new( 1, 1, 1, 0.9 )
MENU_BUTTON_SELECTED_COLOR = Vec4.new( 0, 0, 1, 0.7 )

MENU_BUTTON_FONT = "fonts/orbitron"

-------------------------------------------------
-- MAIN TITLE
-------------------------------------------------

MainTitle = Window.new()
MainTitle.rect = Rectangle.new( 360, 40, 280, 60 )
MainTitle.noevents = true

-- FIXME does not work if video in the desktopWindow.background
MainTitle.backcolor = Vec4.new( 0, 0, 0, 0.7 )

--MainTitle.TestFunc = TestFunc
--function MainTitle.OnTest()
--	print( "MainTitle:OnTest()" )
--end


-- MainTitle_Text = Window.new
-- {
	-- text = "Tekuum";
-- }
MainTitle_Text = Window.new()
MainTitle_Text.rect = Rectangle.new( 0, 0, 280, 60 );
MainTitle_Text.text = "Tekuum"
MainTitle_Text.font = "fonts/orbitron"
MainTitle_Text.forecolor = Vec4.new( 1, 1, 1, 0.9 )
MainTitle_Text.textscale = 0.9
MainTitle_Text.textalignx = 10
MainTitle_Text.textaligny = -5

MainTitle_TextBlur = MainTitle_Text:clone()
MainTitle_TextBlur.rect = Rectangle.new( 5, 0, 280, 60 )
MainTitle_TextBlur.forecolor = Vec4.new( 1, 1, 1, 0.2 )

MainTitle_TextBlur2 = MainTitle_Text:clone()
MainTitle_TextBlur2.rect = Rectangle.new( 10, 0, 280, 60 )
MainTitle_TextBlur2.forecolor = Vec4.new( 1, 1, 1, 0.1 )

MainTitle:AddChildren( MainTitle_Text, MainTitle_TextBlur, MainTitle_TextBlur2 )


-------------------------------------------------
-- INFORMATION BOX
-------------------------------------------------

Tooltip = Window.new()
Tooltip.rect = Rectangle.new( 30, 400, 580, 30 );
--Tooltip.text = "test"
Tooltip.backcolor = MENU_BUTTON_BACK_COLOR
Tooltip.visible = false
Tooltip.noevents = true


Tooltip_Icon = Window.new()
Tooltip_Icon.rect = Rectangle.new( 5, 5, 20, 20 )
Tooltip_Icon.background = "guis/assets/mainmenu/tooltip"
Tooltip_Icon.backcolor = Vec4.new( 1, 1, 1, 0 )

Tooltip_Label = Window.new()
Tooltip_Label.rect = Rectangle.new( 30, 0, 490, 30 )
Tooltip_Label.forecolor = MENU_BUTTON_FORE_COLOR
--Tooltip_Label.text = "toolbox"
Tooltip_Label.textscale = 0.2
Tooltip_Label.font = "fonts/orbitron"

Tooltip:AddChildren( Tooltip_Icon, Tooltip_Label )



-------------------------------------------------
-- MAIN MENU
-------------------------------------------------

function CreateMenuButton( x, y, width, height, text )
	local menuButton = Window.new()
	menuButton.rect = Rectangle.new( x, y, width, height );
	menuButton.text = text
	menuButton.font = MENU_BUTTON_FONT
	menuButton.backcolor = MENU_BUTTON_BACK_COLOR
	menuButton.forecolor = MENU_BUTTON_FORE_COLOR
	--menuButton.textscale = 0.9
	menuButton.textalignx = 5
	menuButton.textaligny = -1
	
	return menuButton
end

MainMenu_Content = Window.new()
MainMenu_Content.rect = Rectangle.new( 60, 150, 200, 260 )

function MainMenu_Content:OnActivate()
	print "MainMenu_Content:OnActivate()"
end

function MainMenu_Content:OnDeactivate()
	print "MainMenu_Content:OnDeactivate()"
end

function MainMenu_Content:OnOpen()
	print "MainMenu_Content:OnOpen()"
	
	MainMenu_StartGameButton:SetFocus()  
end

--menuY = 50
function ResetMenuY( y )
	menuY = y
	return y
end

function AdvanceMenuY()
	MENU_Y_OFFSET = 4
	menuY = menuY + 26 + MENU_Y_OFFSET
	
	return menuY
end
	

MainMenu_Title = CreateMenuButton( 0, 15, 200, 30, "main_" )
MainMenu_Title.textscale = 0.45
MainMenu_Title.textalign = 2 -- right
MainMenu_Title.noevents = 1

MainMenu_StartGameButton = CreateMenuButton( 0, ResetMenuY( 50 ), 200, 26, "start game" )

-- MainMenu_StartGameButton.frames = -1

-- -- performance test
-- function MainMenu_StartGameButton:OnFrame()
	-- --print( "MainMenu_StartGameButton.OnFrame" )
	
	-- if( self.frames < 60 ) then
		
		-- --self.color.a = ( self.frames / 60.0 )
		-- --self.backcolor = self.color
		-- local color = Vec4.new( 0, 0, 0, ( self.frames / 60.0 ) )
		-- self.backcolor = color
		-- color = nil
		-- self.frames = self.frames + 1 
	-- else
		-- self.frames = 0
		
		-- --print( collectgarbage( "count" ) ) 
		-- collectgarbage() 
		-- print( collectgarbage( "count" ) )
	-- end
	
	-- --print( self.backcolor ) 
-- end

function MainMenu_StartGameButton:OnFocusGain()
	--print( "MainMenu_StartGameButton:OnFocusGain()" )
	
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
	
	Tooltip_Label.text = "Start a new single player campaign"
	Tooltip.visible = true
end

function MainMenu_StartGameButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
	
	Tooltip.visible = false;
end

-- function MainMenu_StartGameButton:OnMouseEnter()
	-- self.backcolor =  MENU_BUTTON_SELECTED_COLOR
-- end
		
-- onMouseExit
-- {
	-- print "MainMenu_StartGameButton.OnMouseExit";
-- }
		
function MainMenu_StartGameButton:OnAction()
	print( "MainMenu_StartGameButton:OnAction()" )
end
	-- }
	
MainMenu_OptionsButton = CreateMenuButton( 0, AdvanceMenuY(), 200, 26, "options" )	

function MainMenu_OptionsButton:OnFocusGain()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function MainMenu_OptionsButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function MainMenu_OptionsButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function MainMenu_OptionsButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function MainMenu_OptionsButton:OnAction()
	MainMenu_Content:Close()
	OptionsMenu_Content:Open()
end



MainMenu_QuitButton = CreateMenuButton( 0, 140, 200, 26, "quit" )	

function MainMenu_QuitButton:OnFocusGain()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function MainMenu_QuitButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function MainMenu_QuitButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function MainMenu_QuitButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function MainMenu_QuitButton:OnAction()
	self:AddCommand( "quit" )
end

MainMenu_Content:AddChildren( MainMenu_Title )
MainMenu_Content:AddChildren( MainMenu_StartGameButton )
MainMenu_Content:AddChildren( MainMenu_OptionsButton )
MainMenu_Content:AddChildren( MainMenu_QuitButton )


-------------------------------------------------
-- OPTIONS MENU
-------------------------------------------------

OptionsMenu_Content = Window.new()
OptionsMenu_Content.rect = Rectangle.new( 60, 150, 200, 260 )
OptionsMenu_Content.visible = false

function OptionsMenu_Content:OnActivate()
	print "OptionsMenu_Content:OnActivate()"
end

function OptionsMenu_Content:OnDeactivate()
	print "OptionsMenu_Content:OnDeactivate()"
end

function OptionsMenu_Content:OnOpen()
	print "OptionsMenu_Content:OnOpen()"
	
	OptionsMenu_VideoButton:SetFocus()  
end


OptionsMenu_Title = CreateMenuButton( 0, 15, 200, 30, "options_" )
OptionsMenu_Title.textscale = 0.45
OptionsMenu_Title.textalign = 2 -- right
OptionsMenu_Title.noevents = 1

OptionsMenu_VideoButton = CreateMenuButton( 0, ResetMenuY( 50 ), 200, 26, "video" )

function OptionsMenu_VideoButton:OnFocusGain()
	--print( "OptionsMenu_VideoButton:OnFocusGain()" )
	
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
	
	Tooltip_Label.text = "Adjust graphics settings"
	Tooltip.visible = true
end

function OptionsMenu_VideoButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
	
	Tooltip.visible = false;
end

function OptionsMenu_VideoButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_VideoButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end
		
function OptionsMenu_VideoButton:OnAction()
	print( "OptionsMenu_VideoButton:OnAction()" )
end


	
OptionsMenu_AudioButton = CreateMenuButton( 0, 80, 200, 26, "audio" )	

function OptionsMenu_AudioButton:OnFocusGain()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_AudioButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_AudioButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_AudioButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_AudioButton:OnAction()
	--self:Close()
	--OptionsMenu_Content:Open()
end



OptionsMenu_ControlsButton = CreateMenuButton( 0, 110, 200, 26, "controls" )	

function OptionsMenu_ControlsButton:OnFocusGain()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_ControlsButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_ControlsButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_ControlsButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_ControlsButton:OnAction()
	--self:Close()
	--OptionsMenu_Content:Open()
end



OptionsMenu_GameButton = CreateMenuButton( 0, 140, 200, 26, "game" )	

function OptionsMenu_GameButton:OnFocusGain()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_GameButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_GameButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_GameButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_GameButton:OnAction()
	--self:Close()
	--OptionsMenu_Content:Open()
end



OptionsMenu_ReturnButton = CreateMenuButton( 0, 200, 200, 26, "back" )	 

function OptionsMenu_ReturnButton:OnFocusGain()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_ReturnButton:OnFocusLose()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_ReturnButton:OnMouseEnter()
	self.backcolor = MENU_BUTTON_SELECTED_COLOR
end

function OptionsMenu_ReturnButton:OnMouseExit()
	self.backcolor = MENU_BUTTON_BACK_COLOR
end

function OptionsMenu_ReturnButton:OnAction()
	OptionsMenu_Content:Close()
	MainMenu_Content:Open()
end

OptionsMenu_Content:AddChildren( OptionsMenu_Title )
OptionsMenu_Content:AddChildren( OptionsMenu_VideoButton )
OptionsMenu_Content:AddChildren( OptionsMenu_AudioButton )
OptionsMenu_Content:AddChildren( OptionsMenu_ControlsButton )
OptionsMenu_Content:AddChildren( OptionsMenu_GameButton ) 
OptionsMenu_Content:AddChildren( OptionsMenu_ReturnButton )


-------------------------------------------------
-- DESKTOP
-------------------------------------------------

function main( desktopWindow )
	print( "--> main( desktopWindow )\n" )
  
	desktopWindow.menugui = true
	desktopWindow.background = "gui/mainmenu/background"
	
	desktopWindow:AddChild( MainTitle )
	desktopWindow:AddChild( Tooltip )
	desktopWindow:AddChild( MainMenu_Content )
	desktopWindow:AddChild( OptionsMenu_Content )
	
	--desktopWindow.text = "Desktop"
	--desktopWindow.TestFunc = TestFunc
	
	MainMenu_Content:Open()
  
	function desktopWindow:OnESC()
		print( "desktopWindow:OnESC()" )
		self:AddCommand( "close" )
	end
  
	function desktopWindow:TestFunc()
		print( "TestFunc: desktopWindow.text = '" .. self.text .. "'" )
		print( "TestFunc: desktopWindow.rect = '" .. self.rect .. "'" )
	end


	--print( collectgarbage( "count" ) * 1024 )
	--collectgarbage()
	--print( collectgarbage( "count" ) * 1024 )

	--print( rect ) 

	print( "<-- main( desktopWindow )" )
	
end

