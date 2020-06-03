package fi.aardsoft.emacsbridge;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class EmacsBridgeBroadcastReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Intent startServiceIntent = new Intent(context, EmacsBridgeBroadcastReceiver.class);
        context.startService(startServiceIntent);
    }
}
