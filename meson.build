project(
    'cinderward',
    'cpp',
    version: '0.1',
    meson_version: '>=1.0.0',
    default_options: ['cpp_std=c++17', 'buildtype=release'],
)

qt6 = import('qt6')
qt = dependency('qt6', version: '>=6.9', modules: ['Core', 'Gui', 'Qml', 'Quick', 'QuickControls2', 'Svg', 'DBus'])
kf6_core = dependency('KF6CoreAddons', version: '>=6.13')
cpp_compiler = meson.get_compiler('cpp')
kf6_i18n = declare_dependency(
    include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KI18n'),
    dependencies: [cpp_compiler.find_library('KF6I18n'), cpp_compiler.find_library('KF6I18nQml')],
)
kf6_windowsystem = dependency('KF6WindowSystem', version: '>=6.13')
mauikit = dependency('MauiKit4', version: '>=4.0.5')

git_program = find_program('git', required: false)
git_branch = 'Stable'
git_commit_hash = '0.0.3'
if git_program.found()
    git_branch_result = run_command(git_program, '-C', meson.project_source_root(), 'rev-parse', '--abbrev-ref', 'HEAD', check: false)
    git_commit_result = run_command(git_program, '-C', meson.project_source_root(), 'rev-parse', '--short', 'HEAD', check: false)
    if git_branch_result.returncode() == 0 and git_branch_result.stdout().strip() != ''
        git_branch = git_branch_result.stdout().strip()
    endif
    if git_commit_result.returncode() == 0 and git_commit_result.stdout().strip() != ''
        git_commit_hash = git_commit_result.stdout().strip()
    endif
endif

cpp_args = ['-DGIT_BRANCH="' + git_branch + '"', '-DGIT_COMMIT_HASH="' + git_commit_hash + '"']
if host_machine.cpu_family() == 'x86_64'
    cpp_args += ['-march=x86-64-v3']
endif

qt_sources = qt6.preprocess(
    qresources: 'resources.qrc',
    moc_headers: ['src/firewallbackend.h'],
)

executable(
    'cinderward',
    ['src/main.cpp', 'src/firewallbackend.cpp'] + qt_sources,
    dependencies: [qt, kf6_core, kf6_i18n, kf6_windowsystem, mauikit],
    cpp_args: cpp_args,
    install: true,
)

install_data('data/org.nitrux.cinderward.desktop', install_dir: get_option('datadir') / 'applications')
