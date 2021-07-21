xvfb = find_program('xvfb-run', native : true, required : false)

# Tests require Xvfb, so do nothing if it's not available
if not xvfb.found()
	warning('Xvfb is required to run tests')
	subdir_done()
endif

test_functional = find_program('test_functional.sh', dirs : [meson.source_root() / 'test'])
test_wrapper = find_program('meson-test-env.sh', dirs : [meson.source_root() / 'test'])

test_env = environment()
test_env.set('OSK_SDL_EXE_PATH', osk_sdl_exe.full_path())
test_env.set('OSK_SDL_CONF_PATH', meson.source_root() / 'osk.conf')

add_test_setup(
	'headless',
	exe_wrapper : test_wrapper,
	is_default : true,
)

test('Functional test - keyscript, physical keyboard input',
	test_functional,
	args : ['test_keyscript_phys'],
	env : test_env,
)

test('Functional test - keyscript, physical keyboard input, with no keyboard option',
	test_functional,
	args : ['test_keyscript_no_keyboard_phys'],
	env : test_env,
)

test('Functional test - keyscript, mouse keyboard input, letters',
	test_functional,
	args : ['test_keyscript_mouse_letters'],
	env : test_env,
)

test('Functional test - keyscript, mouse keyboard input, symbols',
	test_functional,
	args : ['test_keyscript_mouse_symbols'],
	env : test_env,
)

test('Functional test - keyscript, mouse keyboard input, osk toggle',
	test_functional,
	args : ['test_keyscript_mouse_toggle_osk'],
	env : test_env,
)

test('Functional test - luks',
	test_functional,
	args : ['test_luks_phys'],
	env : test_env,
)
