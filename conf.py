import dbus

bus = dbus.SessionBus()
proxy = bus.get_object('org.example.LauncherDaemon', '/org/example/LauncherDaemon')
interface = dbus.Interface(proxy, 'org.example.LauncherDaemon')
interface.ShowLauncher()
