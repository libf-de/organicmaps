package app.organicmaps.car.screens;

import android.location.Location;
import android.os.SystemClock;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.car.app.CarContext;
import androidx.car.app.model.Action;
import androidx.car.app.model.ActionStrip;
import androidx.car.app.model.CarIcon;
import androidx.car.app.model.Template;
import androidx.car.app.navigation.model.NavigationTemplate;
import androidx.core.graphics.drawable.IconCompat;
import androidx.lifecycle.LifecycleOwner;
import app.organicmaps.car.R;
import app.organicmaps.car.util.UiHelpers;
import app.organicmaps.sdk.OrganicMaps;
import app.organicmaps.sdk.Router;
import app.organicmaps.sdk.car.renderer.Renderer;
import app.organicmaps.sdk.car.screens.BaseMapScreen;
import app.organicmaps.sdk.Framework;
import app.organicmaps.sdk.location.LocationListener;
import app.organicmaps.sdk.sound.MediaPlayerWrapper;
import app.organicmaps.sdk.util.StringUtils;

public class FreeDriveScreen extends BaseMapScreen
{
  @NonNull
  private final LocationListener mLocationListener = this::updateSpeedLimit;

  @Nullable
  private MediaPlayerWrapper mSpeedingWarnPlayer;
  private long mLastSpeedingWarnMs = 0;

  public FreeDriveScreen(@NonNull CarContext carContext, @NonNull OrganicMaps organicMapsContext,
                         @NonNull Renderer surfaceRenderer)
  {
    super(carContext, organicMapsContext, surfaceRenderer);
  }

  @NonNull
  @Override
  protected Template onGetTemplateImpl()
  {
    final NavigationTemplate.Builder builder = new NavigationTemplate.Builder();
    builder.setMapActionStrip(
        UiHelpers.createMapActionStrip(getCarContext(), getSurfaceRenderer(), getLocationHelper()));
    builder.setActionStrip(createActionStrip());

    return builder.build();
  }

  @Override
  public void onCreate(@NonNull LifecycleOwner owner)
  {
    super.onCreate(owner);
    mSpeedingWarnPlayer = new MediaPlayerWrapper(getCarContext());
    getLocationHelper().addListener(mLocationListener);
    updateSpeedLimit(/* location */ null);
  }

  @Override
  public void onDestroy(@NonNull LifecycleOwner owner)
  {
    super.onDestroy(owner);
    getLocationHelper().removeListener(mLocationListener);
    getSurfaceRenderer().setSpeedLimit(0, false);
    if (mSpeedingWarnPlayer != null)
    {
      mSpeedingWarnPlayer.release();
      mSpeedingWarnPlayer = null;
    }
  }

  private void updateSpeedLimit(@Nullable Location location)
  {
    if (Router.get() != Router.Vehicle)
    {
      getSurfaceRenderer().setSpeedLimit(0, false);
      return;
    }
    final double speedLimitMps = Framework.nativeGetFreeRoamSpeedLimitMps();
    if (speedLimitMps < 0)
    {
      getSurfaceRenderer().setSpeedLimit(0, false);
      return;
    }
    final boolean alert = location != null && speedLimitMps > 0 && speedLimitMps < location.getSpeed();
    getSurfaceRenderer().setSpeedLimit(StringUtils.nativeFormatSpeed(speedLimitMps), alert);

    if (alert && mSpeedingWarnPlayer != null)
    {
      final int toleranceKmh = Framework.nativeGetSpeedLimitWarningToleranceKmh();
      if (toleranceKmh >= 0)
      {
        final double toleranceMps = toleranceKmh / 3.6;
        if (location.getSpeed() > speedLimitMps + toleranceMps)
        {
          final long now = SystemClock.elapsedRealtime();
          if (now - mLastSpeedingWarnMs >= 30_000L)
          {
            mLastSpeedingWarnMs = now;
            mSpeedingWarnPlayer.playback(app.organicmaps.routing.R.raw.speed_cams_beep);
          }
        }
      }
    }
  }

  @NonNull
  private ActionStrip createActionStrip()
  {
    final Action.Builder finishActionBuilder = new Action.Builder();
    finishActionBuilder.setIcon(
        new CarIcon.Builder(IconCompat.createWithResource(getCarContext(), R.drawable.ic_close)).build());
    finishActionBuilder.setOnClickListener(this::finish);

    final ActionStrip.Builder builder = new ActionStrip.Builder();
    builder.addAction(finishActionBuilder.build());
    builder.addAction(UiHelpers.createSettingsAction(this, getSurfaceRenderer()));
    return builder.build();
  }
}
