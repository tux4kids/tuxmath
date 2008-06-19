MACRELEASEDIR = build/Release
MACBUNDLE = $(MACRELEASEDIR)/TuxMath.app
FRAMEWORKDIR = $(MACBUNDLE)/Contents/Frameworks/

clean:
	cd build && rm -r *

macapp:
	xcodebuild && echo "Creating dmg in $(MACRELEASEDIR)" && hdiutil create  -ov -srcfolder $(MACBUNDLE) -volname TuxMath -imagekey zlib-level=6 $(MACRELEASEDIR)/TuxMath.dmg

