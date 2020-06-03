package fi.aardsoft.emacsbridge;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;

public class EmacsBridgeNotification {
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;

    public EmacsBridgeNotification() {}

    public static void notify(Context context, String message, String title) {
      try {
        m_notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        int importance = NotificationManager.IMPORTANCE_DEFAULT;
        NotificationChannel notificationChannel
          = new NotificationChannel("EmacsBridge", "Emacs bridge", importance);

        notificationChannel.enableVibration(true);
        notificationChannel.enableLights(false);
        // TODO: add setSound

        m_notificationManager.createNotificationChannel(notificationChannel);

        m_builder = new Notification.Builder(context, notificationChannel.getId());

        m_builder.setSmallIcon(R.drawable.icon)
          .setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.icon))
          .setContentTitle(title)
          .setContentText(message)
          .setAutoCancel(true);

        m_notificationManager.notify(0, m_builder.build());
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
}
