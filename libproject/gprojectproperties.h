#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <QString>
#include <QPair>
#include <QList>

struct GProjectProperties
{
    GProjectProperties()
	{
		clear();
	}

	void clear()
	{
		// graphics options
		scaleMode = 0;
		logicalWidth = 320;
		logicalHeight = 480;
		imageScales.clear();
		orientation = 0;
		fps = 60;

		// iOS options
        retinaDisplay = 0;
		autorotation = 0;

        // input options
        mouseToTouch = true;
        touchToMouse = true;
        mouseTouchOrder = 0;

        // export options
		architecture = 0;
		assetsOnly = false;
		iosDevice = 0;
		packageName = "com.yourdomain.yourapp";
        encryptCode = false;
        encryptAssets = false;
    }

	// graphics options
	int scaleMode;
	int logicalWidth;
	int logicalHeight;
    QList<QPair<QString, double> > imageScales;
	int orientation;
	int fps;

	// iOS options
    int retinaDisplay;
	int autorotation;

    // input options
    bool mouseToTouch;
    bool touchToMouse;
    int mouseTouchOrder;

    // export options
	int architecture;
	bool assetsOnly;
	int iosDevice;
	QString packageName;
    bool encryptCode;
    bool encryptAssets;
};

#endif
