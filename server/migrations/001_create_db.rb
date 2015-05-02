Sequel.migration do
  up do
    create_table(:envs) do
      primary_key :id
      Integer :datetime
      Float :temperature
      Float :humidity
      Integer :brightness
    end
  end

  down do
    drop_table(:envs)
  end
end

