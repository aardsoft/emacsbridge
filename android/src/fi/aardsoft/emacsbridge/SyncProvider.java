package fi.aardsoft.emacsbridge;

import android.database.Cursor;
import android.net.Uri;
import android.content.ContentProvider;
import android.content.ContentValues;

// a stub provider based on
// https://developer.android.com/training/sync-adapters/creating-stub-provider
// might be used as proper provider eventually.
public class SyncProvider extends ContentProvider {
    /*
     * Always return true, indicating that the
     * provider loaded correctly.
     */
    @Override
    public boolean onCreate() {
      return true;
    }
    /*
     * Return no type for MIME type
     */
    @Override
    public String getType(Uri uri) {
      return null;
    }
    /*
     * query() always returns no results
     *
     */
    @Override
    public Cursor query(
      Uri uri,
      String[] projection,
      String selection,
      String[] selectionArgs,
      String sortOrder) {
      return null;
    }
    /*
     * insert() always returns null (no URI)
     */
    @Override
    public Uri insert(Uri uri, ContentValues values) {
      return null;
    }
    /*
     * delete() always returns "no rows affected" (0)
     */
    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
      return 0;
    }
    /*
     * update() always returns "no rows affected" (0)
     */
    public int update(
      Uri uri,
      ContentValues values,
      String selection,
      String[] selectionArgs) {
      return 0;
    }
}
