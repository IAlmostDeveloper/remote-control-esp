package ru.ialmostdeveloper.remotecontrol.controllers;

import java.util.List;

public class NECController implements IController {

    private List<ControllerButton> controlButtons;
    private String deviceId;
    private final String className = "NECController";
    private String name;
    public NECController(String deviceId, String name, List<ControllerButton> controlButtons) {
        this.deviceId = deviceId;
        this.name = name;
        setControlButtons(controlButtons);
    }

    @Override
    public List<ControllerButton> getControlButtons() {
        return controlButtons;
    }

    @Override
    public void setControlButtons(List<ControllerButton> controlButtons) {
        this.controlButtons = controlButtons;
    }

    @Override
    public void addControllerButton(ControllerButton button) {
        controlButtons.add(button);
    }

    @Override
    public void removeControllerButton(String name) {
        for(ControllerButton button : controlButtons)
            if(button.name.equals(name)){
                controlButtons.remove(button);
                return;
            }
    }

    @Override
    public long getControlButtonCode(String name) {
        for (ControllerButton button : controlButtons) {
            if (button.name.equals(name))
                return button.code;
        }
        throw new IllegalArgumentException("Key is not in List: " + name);
    }

    public String getDeviceId() {
        return deviceId;
    }

    @Override
    public String getName() {
        return name;
    }


    @Override
    public String getClassName() {
        return className;
    }
}
