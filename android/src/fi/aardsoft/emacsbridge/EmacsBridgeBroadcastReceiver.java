package fi.aardsoft.emacsbridge;

import android.os.Build;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class EmacsBridgeBroadcastReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Intent startServiceIntent = new Intent(context, EmacsBridgeService.class);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
          context.startForegroundService(new Intent(context, EmacsBridgeService.class));
        } else {
          context.startService(startServiceIntent);
        }
    }
}
