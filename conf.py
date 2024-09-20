import dbus

bus = dbus.SessionBus()
proxy = bus.get_object('org.example.PopupDaemon', '/org/example/PopupDaemon')
interface = dbus.Interface(proxy, 'org.example.PopupDaemon')

interface.SetLabel("hello")
