## Що це

Збірка **WSJT-X** з інтеграцією [uahamaward.com](https://uahamaward.com). Усе інше — оригінальний WSJT-X, без змін.

Це **не офіційний реліз WSJT-X**. Про проблеми саме з тим, що описано нижче, пишіть сюди, а не команді WSJT-X.

## Що додано

**Фільтр країн (Налаштування → UaHam Filter).** Обирайте країни зі списку DXCC у двох режимах: *ховати обрані* або *показувати лише обрані*. Прихована станція не з'являється ні у вікні активності діапазону, ні у вікні частоти прийому, ні в переліку активних станцій — і автопослідовність не відповість їй навіть на прямий виклик. `ALL.TXT` зберігає всі декоди, а програми, що читають WSJT-X по UDP (JTAlert, GridTracker), бачать усе.

На відміну від наявної вкладки фільтрів, де дванадцять текстових полів шукають збіг у тексті повідомлення, цей фільтр питає `cty.dat` — тому «лише Японія» працює для всіх префіксів JA, JE–JS, 7J–7N, 8J–8N разом, включно зі складеними позивними.

**Пряме з'єднання з сайтом (Налаштування → UaHam Site).** WSJT-X сам віддає кожен записаний зв'язок сторінкам нагород на uahamaward.com — байдуже, записали ви його вручну чи це зробила автопослідовність. **Окрема програма-місток більше не потрібна:** нічого не треба завантажувати й нічого оновлювати. Якщо жодну сторінку журналу не відкрито, нічого не надсилається, і запис зв'язку працює точно так, як завжди.

**Дані позивного з QRZ.com (Налаштування → QRZ).** Вкладка «Call info» у головному вікні показує ім'я, адресу, країну й локатор станції, яка стоїть у полі DX Call, — щоб вітатися на ім'я, а не з позивним. Потрібен ваш власний обліковий запис QRZ.com із передплатою XML; без нього вкладка порожня й у мережу нічого не йде. Дані показуються вам і нікуди не передаються.

**Український інтерфейс (Налаштування → Language).** Перекладено головне вікно, меню, всі вкладки налаштувань разом із підказками, запис QSO та вікна помилок. Назви режимів, діапазонів, одиниці та ефірні скорочення лишилися англійською навмисно. Мова діє з наступного запуску.

**Вкладка «UaHam» у головному вікні** показує режим фільтра, скільки декодів приховано, чи підключений браузер і скільки QSO надіслано. Те саме коротко — в рядку стану.

## Виправлено в успадкованому коді

Прапорець фільтрації скидався один раз на пачку декодів замість кожного декоду. Через це варто було відфільтрувати один сигнал у періоді — і зникали всі наступні в тій самій пачці, разом з автовідповіддю.

## Встановлення

Windows — `.exe`. macOS — `.pkg` під ваш процесор (збірки не підписані: перший запуск через Ctrl+клік → «Відкрити»). Linux — `.AppImage`, `.deb` або `.rpm`.

---

# English

## What this is

A build of **WSJT-X** with [uahamaward.com](https://uahamaward.com) integration. Everything else is stock WSJT-X, unchanged.

This is **not an official WSJT-X release**. Report problems with what is described below here, not to the WSJT-X team.

## What was added

**Country filter (Settings → UaHam Filter).** Pick DXCC entities from a list, in two modes: *hide the chosen ones* or *show only those*. A hidden station appears in neither Band Activity, nor Rx Frequency, nor Active Stations — and auto-sequencing will not answer it even when it calls you directly. `ALL.TXT` keeps every decode, and programs reading WSJT-X over UDP (JTAlert, GridTracker) see everything.

Unlike the existing Filters tab, where twelve text boxes are matched against words of the message, this one asks `cty.dat` — so "Japan only" covers JA, JE–JS, 7J–7N and 8J–8N together, compound callsigns included.

**Direct connection to the site (Settings → UaHam Site).** WSJT-X hands every logged contact to the award pages on uahamaward.com itself, whether you logged it by hand or auto-sequencing did. **The separate bridge program is no longer needed:** nothing to download, nothing to keep updated. With no log page open nothing is sent, and logging works exactly as it always did.

**QRZ.com callsign lookup (Settings → QRZ).** A "Call info" tab in the main window shows the name, address, country and grid of whoever is in the DX Call box — so you can greet an operator by name rather than by callsign. It needs your own QRZ.com account with an XML subscription; without one the tab says so and no request is ever made. The data is shown to you and goes nowhere else.

**Ukrainian interface (Settings → Language).** The main window, menus, every settings tab including its tooltips, the QSO logging dialog and error windows are translated. Mode names, band names, units and on-air abbreviations are deliberately left in English. The language takes effect on the next start.

**A "UaHam" tab in the main window** shows the filter mode, how many decodes were hidden, whether a browser is connected and how many QSOs were sent. The same in short in the status bar.

## Fixed in inherited code

The filtering flag was cleared once per batch of decodes instead of once per decode. One filtered signal in a period was therefore enough to hide every later decode in the same batch, and to suppress auto-sequencing with them.

## Installing

Windows — `.exe`. macOS — a `.pkg` for your processor (the builds are unsigned: on first run use Ctrl+click → Open). Linux — `.AppImage`, `.deb` or `.rpm`.
