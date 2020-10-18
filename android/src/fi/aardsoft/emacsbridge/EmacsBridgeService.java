package fi.aardsoft.emacsbridge;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtService;

public class EmacsBridgeService extends QtService {
    private static final String TAG = "EmacsBridgeService";
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;

    @Override
    public void onCreate() {
      super.onCreate();
      Log.i(TAG, "Creating Service");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Destroying Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
      m_notificationManager =
        (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

      Context context = getApplicationContext();
      Intent showTaskIntent =
        new Intent(context,
                   org.qtproject.qt5.android.bindings.QtActivity.class);
      showTaskIntent.setAction(Intent.ACTION_MAIN);
      showTaskIntent.addCategory(Intent.CATEGORY_LAUNCHER);
      showTaskIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
      PendingIntent contentIntent =
        PendingIntent.getActivity(context, 0, showTaskIntent, PendingIntent.FLAG_UPDATE_CURRENT);

      int importance = NotificationManager.IMPORTANCE_DEFAULT;
      NotificationChannel notificationChannel =
        new NotificationChannel("EmacsBridgeService", "Emacs bridge", importance);

        m_notificationManager.createNotificationChannel(notificationChannel);
        m_builder = new Notification.Builder(context, notificationChannel.getId());

        m_builder.setSmallIcon(R.drawable.icon)
          .setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.icon))
          .setTicker("Emacs bridge service")
          .setContentTitle("Emacs bridge service")
          .setContentText("Emacsbridge is running. Tap to open UI.")
          .setContentIntent(contentIntent);

        startForeground(99, m_builder.build());
        return Service.START_NOT_STICKY;
    }

    // handler for starting the service from Qt
    public static void startEmacsBridgeService(Context context) {
      context.startService(new Intent(context, EmacsBridgeService.class));
    }
}
