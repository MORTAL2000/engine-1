local gui = require "ui.shared"

function hud()
	-- todo transparent color background
	gui.window('hud', -1, 80, function ()
		gui.windowPushButton('Quit', 'quit')
	end)
end
