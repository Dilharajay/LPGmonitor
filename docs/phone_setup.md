# Mobile Phone Setup Guide

Setting up your Smart LPG Monitor on your phone is very straightforward! There are two ways you can monitor your LPG cylinder from your mobile device:

### 1. Using the Home Assistant App (Recommended for remote access)
Since you've integrated it with Home Assistant, the best way to view it is through their official app.
1. Download the **Home Assistant Companion App** from the [Google Play Store (Android)](https://play.google.com/store/apps/details?id=io.homeassistant.companion.android) or the [App Store (iOS)](https://apps.apple.com/us/app/home-assistant/id1099568401).
2. Open the app. It will automatically scan your Wi-Fi network for your Home Assistant server.
3. Tap on your server, log in with your Home Assistant credentials, and you're done!
4. The dashboard with the custom sensors you created in `configuration.yaml` will appear just as it does on your computer. Plus, you will receive push notifications if you set up Home Assistant automations for gas leaks!

### 2. Using the Device's Local Web Dashboard
If you enabled the local web dashboard (`web on` in the Serial CLI), you don't even need Home Assistant to view it.
1. Ensure your phone is connected to the **same Wi-Fi network** as the Smart LPG Monitor.
2. Open any web browser on your phone (Chrome, Safari, etc.).
3. Type the ESP8266's IP address directly into the address bar (e.g., `http://192.168.1.100`). You can find this IP address in the Serial Monitor when the device first connects to Wi-Fi.
4. The dark-themed, mobile-responsive dashboard will load directly on your phone!

*Tip: You can "Add to Home Screen" from your phone's browser menu so you can launch the local web dashboard just like an app.*
