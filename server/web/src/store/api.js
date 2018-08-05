import axios from 'axios'

export default {
    envs() {
        return axios.get('/api/v1/envs')
    }
}
