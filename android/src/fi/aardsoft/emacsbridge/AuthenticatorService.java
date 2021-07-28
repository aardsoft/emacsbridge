package fi.aardsoft.emacsbridge;

import android.app.Service;
import android.accounts.Account;
import android.content.Intent;
import android.os.IBinder;

import fi.aardsoft.emacsbridge.Authenticator;
// based on
// https://developer.android.com/training/sync-adapters/creating-authenticator
/**
 * A bound Service that instantiates the authenticator
 * when started.
 */
public class AuthenticatorService extends Service {
    // Instance field that stores the authenticator object
    private Authenticator mAuthenticator;
    @Override
    public void onCreate() {
      // Create a new authenticator object
      mAuthenticator = new Authenticator(this);
    }
    /*
     * When the system binds to this Service to make the RPC call
     * return the authenticator's IBinder.
     */
    @Override
    public IBinder onBind(Intent intent) {
      return mAuthenticator.getIBinder();
    }
}
