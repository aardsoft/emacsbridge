package fi.aardsoft.emacsbridge;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.util.Log;
import java.util.ArrayList;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;
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

    // call intents as defined in JSON similar to the following:
    // {
    //   "package": "com.termux",
    //   "class": "com.termux.app.RunCommandService",
    //   "extra": [
    //     {
    //       "type": "string",
    //       "key": "com.termux.RUN_COMMAND_PATH",
    //       "value": "/data/data/com.termux/files/usr/bin/ps"
    //     },
    //     {
    //       "type": "string",
    //       "key": "com.termux.RUN_COMMAND_ARGUMENTS",
    //       "value": "-a"
    //     }
    //   ],
    //   "action": "com.termux.RUN_COMMAND"
    // }
    //
    // The matching lisp would be:
    //
    // (emacsbridge-post-json "intent" (json-encode `((:package . "com.termux")
    //                                                (:class . "com.termux.app.RunCommandService")
    //                                                (:extra . (((:type . "string")
    //                                                            (:key . "com.termux.RUN_COMMAND_PATH")
    //                                                            (:value . "/data/data/com.termux/files/usr/bin/ps"))
    //                                                           ((:type . "string")
    //                                                            (:key . "com.termux.RUN_COMMAND_ARGUMENTS")
    //                                                            (:value . "-a"))))
    //                                                (:action . "com.termux.RUN_COMMAND"))))
    public static void callIntentFromJson(Context context, String jsonData){
      try {
        JSONObject json = new JSONObject(jsonData);
        Intent intent = new Intent();
        // this is required as that call will always come from a service context
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        if (!json.isNull("action")){
          intent.setAction(json.getString("action"));
        }

        if (!json.isNull("data")){
          intent.setData(Uri.parse(json.getString("data")));
        }

        if (!json.isNull("package") && !json.isNull("class")){
          ComponentName componentName =
            new ComponentName(json.getString("package"), json.getString("class"));
          intent.setComponent(componentName);
        }

        if (!json.isNull("extra")){
          JSONArray extra = json.getJSONArray("extra");
          for (int i=0;i<extra.length();i++){
            JSONObject e = extra.getJSONObject(i);
            if (!e.isNull("type") && !e.isNull("key") && !e.isNull("value")){
              String type = e.getString("type");
              String key = e.getString("key");
              if (type.equals("string")){
                intent.putExtra(key, e.getString("value"));
              } else if (type.equals("stringarray")){
                JSONArray value = e.getJSONArray("value");
                String[] valueArray = new String[value.length()];
                for(int j=0;j<value.length();j++){
                  Log.i(TAG, "Adding " + value.getJSONObject(j).getString("value")
                        + " for key " + key);
                  valueArray[j] = value.getJSONObject(j).getString("value");
                }
                intent.putExtra(key, valueArray);
              } else if (type.equals("arrayliststring")){
                JSONArray value = e.getJSONArray("value");
                ArrayList<String> valueList = new ArrayList<String>();
                for(int j=0;j<value.length();j++){
                  Log.i(TAG, "Adding " + value.getJSONObject(j).getString("value")
                        + " for key " + key);
                  valueList.add(value.getJSONObject(j).getString("value"));
                }
                intent.putStringArrayListExtra(key, valueList);
              } else if (type.equals("boolean")){
                intent.putExtra(key, e.getBoolean("value"));
              } else if (type.equals("double")){
                intent.putExtra(key, e.getInt("double"));
              } else if (type.equals("int")){
                intent.putExtra(key, e.getInt("value"));
              } else if (type.equals("long")){
                intent.putExtra(key, e.getLong("value"));
              }
            }
          }
        }

        if (!json.isNull("startType")){
          String startType = json.getString("startType");
          if (startType.equals("activity")){
            Log.i(TAG, "Starting intent as activity");
            context.startActivity(intent);
          } else if (startType.equals("service")){
            Log.i(TAG, "Starting intent as service");
            context.startService(intent);
          } else {
            Log.i(TAG, "Unhandled start type: " + startType);
          }
        } else
          context.startActivity(intent);
      } catch (JSONException e) {
        // TODO
      }
    }

    // If intentData is present it needs to be a valid string representation of
    // a URI, so possible something like package: needs to be pre-pended
    // this is a convenience function for calling simple intents from the UI
    // part of the application. Anything requiring additional data should go
    // through the json call.
    // Not that intentData needs to be a valid string representation of a URL,
    // so possibly something like package: needs to be pre-pended
    public static void callIntent(Context context,
                                  String intentAction,
                                  String intentData,
                                  String intentPackage,
                                  String intentClass){
      Intent intent = new Intent();
      if (!intentAction.isEmpty()){
        intent.setAction(intentAction);
      }
      if (!intentData.isEmpty()){
        intent.setData(Uri.parse(intentData));
      }
      if (!intentPackage.isEmpty() && !intentClass.isEmpty()){
        ComponentName componentName =
          new ComponentName(intentPackage, intentClass);
        intent.setComponent(componentName);
      }
      context.startActivity(intent);
    }

    public static void openAppSettings(Context context){
      callIntent(context,
                 android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS,
                 "package:" + context.getPackageName(),
                 "", "");
    }
}
