import re

with open('d:/Unbound/pokemon/cpp/client/main.cpp', 'r') as f:
    content = f.read()

# Remove virtual input merge from input loop
input_search = '''
            auto gs = dynamic_cast<unboundmp::ui::GameScreen*>(ui_engine.GetScreens().GetCurrentScreen());
            if (gs) {
                InputState virtual_in = gs->GetVirtualInput();
                input.held_mask |= virtual_in.held_mask;
            }
'''
content = content.replace(input_search, '')

with open('d:/Unbound/pokemon/cpp/client/main.cpp', 'w') as f:
    f.write(content)
