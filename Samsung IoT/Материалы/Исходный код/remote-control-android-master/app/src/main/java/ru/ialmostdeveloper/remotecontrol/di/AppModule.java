package ru.ialmostdeveloper.remotecontrol.di;

import android.content.Context;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;
import retrofit2.Retrofit;
import retrofit2.converter.gson.GsonConverterFactory;
import ru.ialmostdeveloper.remotecontrol.controllers.ControllerButton;
import ru.ialmostdeveloper.remotecontrol.mqtt.Storage;
import ru.ialmostdeveloper.remotecontrol.network.APIService;
import ru.ialmostdeveloper.remotecontrol.network.RequestsManager;
import ru.ialmostdeveloper.remotecontrol.network.Session;

@Module
class AppModule {
    AppModule() {

    }

    private List<ControllerButton> RC5StandardControls() {
        List<ControllerButton> controls = new ArrayList<>();
        controls.add(new ControllerButton("TurnOn", 0x80C));
        controls.add(new ControllerButton("Channel1", 0x801));
        controls.add(new ControllerButton("Channel2", 0x802));
        controls.add(new ControllerButton("Channel3", 0x803));
        controls.add(new ControllerButton("Channel4", 0x804));
        controls.add(new ControllerButton("Channel5", 0x805));
        controls.add(new ControllerButton("Channel6", 0x806));
        controls.add(new ControllerButton("Channel7", 0x807));
        controls.add(new ControllerButton("Channel8", 0x808));
        controls.add(new ControllerButton("Channel9", 0x809));
        controls.add(new ControllerButton("Channel0", 0x800));
        controls.add(new ControllerButton("Previous", 0x821));
        controls.add(new ControllerButton("Next", 0x820));
        return controls;
    }

    @Provides
    @Singleton
    RequestsManager provideRequestsManager(APIService service){
        return new RequestsManager(service);
    }

    @Provides
    @Singleton
    Retrofit provideRetrofit(){
        return new Retrofit.Builder()
                .addConverterFactory(GsonConverterFactory.create())
                .baseUrl("https://ik.remzalp.ru")
                .build();
    }

    @Provides
    @Singleton
    Gson provideGson(){
        return new GsonBuilder().create();
    }

    @Provides
    @Singleton
    APIService provideAPIService(Retrofit retrofit){
        return retrofit.create(APIService.class);
    }

    @Provides
    @Singleton
    Session provideSession(Storage storage){
        return storage.readSession();
    }

    @Provides
    @Singleton
    HashMap<String, List<ControllerButton>> provideControllerPresets(final List<ControllerButton> RC5controlButtons) {
        return new HashMap<String, List<ControllerButton>>() {
            {
                put("RC5", RC5controlButtons);
                put("None", new ArrayList<ControllerButton>());
                put("NEC", new ArrayList<ControllerButton>());
            }
        };
    }

    @Provides
    @Singleton
    List<ControllerButton> provideRC5StandardControls() {
        return RC5StandardControls();
    }

    @Provides
    @Singleton
    Storage provideStorage(Context context) {
        return new Storage(context);
    }

}
