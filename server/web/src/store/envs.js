import api from './api'

export default {
    namespaced: true,
    state: {
        temperature: '--',
        humidity: '--'
    },
    actions: {
        async update(context) {
            let result = await api.envs()
            context.commit('setTemperature', result.data.temperature)
            context.commit('setHumidity', result.data.humidity)
        }
    },
    getters: {
        temperature(state) { return state.temperature },
        humidity(state) { return state.humidity }
    },
    mutations: {
        setTemperature(state, value) {
            state.temperature = value
        },
        setHumidity(state, value) {
            state.humidity = value
        }
    },
}
