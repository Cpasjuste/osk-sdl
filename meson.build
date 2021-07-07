project(
	'osk-sdl',
	'cpp',
	version : '0.65',
	default_options : ['warning_level=3', 'cpp_std=c++2a'],
	meson_version : '>=0.53.0',
)

add_project_arguments('-DVERSION="@0@"'.format(meson.project_version()), language : ['cpp'])

src = [
	'src/config.cpp',
	'src/draw_helpers.cpp',
	'src/keyboard.cpp',
	'src/luksdevice.cpp',
	'src/main.cpp',
	'src/tooltip.cpp',
	'src/util.cpp',
]

man_files = [
	'doc/osk-sdl.1',
	'doc/osk.conf.5',
]

install_data(sources : 'osk.conf', install_dir : get_option('sysconfdir'))

scdoc = dependency('scdoc')
scdoc_prog = find_program(scdoc.get_pkgconfig_variable('scdoc'), native : true)
sh = find_program('sh', native : true)

foreach file : man_files
	filename = file + '.scd'
	section = file.split('.')[-1]
	topic = file.split('.' + section)[-2].split('/')[-1]
	output = '@0@.@1@'.format(topic, section)

	custom_target(
		output,
		input : filename,
		output : output,
		capture : true,
		command : [sh, '-c', scdoc_prog.path() + ' < @INPUT@'],
		install : true,
		install_dir : get_option('mandir') / 'man' + section
	)
endforeach

osk_sdl_exe = executable(
	'osk-sdl',
	src,
	dependencies : [
		dependency('SDL2'),
		dependency('SDL2_ttf'),
		dependency('libcryptsetup'),
	],
	install : true
)

subdir('test')
