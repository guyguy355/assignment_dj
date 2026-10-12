# מתחילים כאן

## לראות את התוצאה בלי להתקין דבר

חלץ את ה־ZIP ופתח בדפדפן את `demo-preview/set-journey.html`.
זה דוח אמיתי שנוצר מהקוד: סדר השירים המקורי מול הצעת סדר לפי BPM, עם פגיעות והחלפות ב־cache.

## להריץ במחשב שלך

ב־Windows פתח את התיקייה ב־VS Code, ובחר טרמינל **Ubuntu / WSL**, לא PowerShell.
אם WSL עדיין לא מותקן, אפשר בינתיים לצפות בדוח המוכן. הפקודות הבאות מיועדות ל־Ubuntu.

אם חסרים קומפיילר או Make, התקן פעם אחת:

```bash
sudo apt update
sudo apt install build-essential
```

פתח טרמינל בתוך תיקיית `DJLab`, זו שבה נמצא `Makefile`, והריץ לפי הסדר:

```bash
pwd
make
make test
make demo
```

כעת ייווצר `report.html`. פתח אותו מסייר הקבצים או מתוך VS Code.

להרצת כל הפלייליסטים לדוגמה:

```bash
./bin/dj_manager demo
```

לבחירת פלייליסט בתפריט:

```bash
./bin/dj_manager interactive
```

לשינוי הדמו ערוך את `examples/night_drive.txt` והריץ שוב `make demo`.
נסה לשנות `controller_cache_size` מ־2 ל־5, או את סדר האינדקסים בשורה `night-drive`.
אל תשנה שמות שירים לכאלה שכבר קיימים: כרגע הכותרת היא המפתח של ה־cache.

## בדיקת זיכרון

```bash
make sanitize
```

הפקודה מפעילה AddressSanitizer ו־UndefinedBehaviorSanitizer, ומבקשת גם בדיקת דליפות.
בסביבת הבדיקה כאן סריקת הדליפות נחסמה על ידי הסביבה. בדיקות ASan/UBSan עברו כשהיא כבויה:

```bash
make sanitize ASAN_OPTIONS=detect_leaks=0
```

זו אינה הוכחה שאין דליפות. הפירוט נמצא ב־`docs/VALIDATION.md`.

## לפני שמציגים בראיון

קרא את `docs/INTERVIEW_HE.md`. חשוב להבין במיוחד למה פגיעה ב־cache חוזרת מיד, מי אחראי למחיקת כל שיר, ואיך clone מונע מצביע תלוי אחרי eviction.

הפרויקט לא מנגן מוזיקה. זו סימולציה לניהול שירים וזיכרון, עם כלי לתכנון סדר לפי BPM.
השינויים לא הועלו ל־GitHub. לפני שילוב בריפו קיים, שמור את העבודה המקומית וסקור את ההבדלים; אל תמחק את היסטוריית Git.
